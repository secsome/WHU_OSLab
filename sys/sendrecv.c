#include <sys/sendrecv.h>
#include <sys/proc.h>

#include <lib/string.h>
#include <lib/assert.h>
#include <lib/syscall.h>

static u32 message_send(process_t* current, int target, message_t* msg);
static u32 message_receive(process_t* current, int source, message_t* msg);

u32 sendrecv_impl(u32 mode, u32 target, message_t* msg, process_t* process)
{
    assert(k_reenter == 0);	// make sure we are not in ring0
	assert((target >= 0 && target < NUM_TASKS + NUM_PROCS) || target == SR_TARGET_ANY || target == SR_TARGET_INTERRUPT);

	int caller = proc2pid(process);
	message_t* msg_la = (message_t*)va2la(caller, msg);
	msg_la->source = caller;

	assert(msg_la->source != target);

	if (mode == SR_MODE_SEND)
		return message_send(process, target, msg);
	else if (mode == SR_MODE_RECEIVE)
		return message_receive(process, target, msg);
	else
		panic("{sendrecv_impl} invalid mode: %d (SR_MODE_SEND : %d, SR_MODE_RECEIVE : %d).", mode, SR_MODE_SEND, SR_MODE_RECEIVE);

	return 0;
}

u32 sendrecv(u32 mode, u32 target, message_t* msg)
{
	u32 ret = 0;

	if (mode == SR_MODE_RECEIVE)
		memset(msg, 0, sizeof(message_t));

	switch (mode)
	{
	case SR_MODE_BOTH:
		ret = lib_sendrecv(SR_MODE_SEND, target, msg);
		if (ret == 0)
			ret = lib_sendrecv(SR_MODE_RECEIVE, target, msg);
		break;
	case SR_MODE_SEND:
	case SR_MODE_RECEIVE:
		ret = lib_sendrecv(mode, target, msg);
		break;
	default:
		assert((mode == SR_MODE_BOTH) || (mode == SR_MODE_SEND) || (mode == SR_MODE_RECEIVE));
		break;
	}

	return ret;
}

static u32 message_send(process_t* current, int target, message_t* msg)
{
	process_t* sender = current;
	process_t* dest = proc_table + target;

	assert(proc2pid(sender) != target);

	if (process_check_deadlock(proc2pid(sender), target))
		panic(">>DEADLOCK<< %s->%s", sender->process_name, dest->process_name);

	// dest is waiting for the msg
	if ((dest->process_flags & SR_STATUS_RECEIVING) && (dest->recvfrom == proc2pid(sender) || dest->recvfrom == SR_TARGET_ANY)) 
	{
		assert(dest->msg);
		assert(msg);

		memcpy(va2la(target, dest->msg), va2la(proc2pid(sender), msg), sizeof(message_t));
		dest->msg = 0;
		dest->process_flags &= ~SR_STATUS_RECEIVING; // dest has received the msg
		dest->recvfrom = SR_TARGET_NONE;
		process_unblock(dest);

		assert(dest->process_flags == 0);
		assert(dest->msg == NULL);
		assert(dest->recvfrom == SR_TARGET_NONE);
		assert(dest->sendto == SR_TARGET_NONE);
		assert(sender->process_flags == 0);
		assert(sender->msg == NULL);
		assert(sender->recvfrom == SR_TARGET_NONE);
		assert(sender->sendto == SR_TARGET_NONE);
	}
	else // dest is not waiting for the msg
	{
		sender->process_flags |= SR_STATUS_SENDING;
		assert(sender->process_flags == SR_STATUS_SENDING);
		sender->sendto = target;
		sender->msg = (message_t*)msg;

		// append to the sending queue
		process_t* p;
		if (dest->sending) {
			p = dest->sending;
			while (p->next_sending)
				p = p->next_sending;
			p->next_sending = sender;
		}
		else
			dest->sending = sender;
		sender->next_sending = NULL;

		process_block(sender);

		assert(sender->process_flags == SR_STATUS_SENDING);
		assert(sender->msg != NULL);
		assert(sender->recvfrom == SR_TARGET_NONE);
		assert(sender->sendto == target);
	}

	return 0;
}

static u32 message_receive(process_t* current, int source, message_t* msg)
{
	process_t* receiver = current;
	process_t* sender = NULL;
	process_t* prev = NULL;
	bool can_copy = false;

	assert(proc2pid(receiver) != source);

	if ((receiver->has_int_msg) && ((source == SR_TARGET_ANY) || (source == SR_TARGET_INTERRUPT)))
	{
		message_t tmp = { 0 };
		tmp.source = SR_TARGET_INTERRUPT;
		tmp.type = SR_MSGTYPE_HARDINT;
		assert(msg);

		memcpy(va2la(proc2pid(receiver), msg), &tmp, sizeof(message_t));

		receiver->has_int_msg = false;

		assert(receiver->process_flags == 0);
		assert(receiver->msg == NULL);
		assert(receiver->sendto == SR_TARGET_NONE);
		assert(receiver->has_int_msg == false);

		return 0;
	}

	// Arrives here if no interrupt for p_who_wanna_recv.
	if (source == SR_TARGET_ANY)
	{
		if (receiver->sending)
		{
			sender = receiver->sending;
			can_copy = true;

			assert(receiver->process_flags == 0);
			assert(receiver->msg == 0);
			assert(receiver->recvfrom == SR_TARGET_NONE);
			assert(receiver->sendto == SR_TARGET_NONE);
			assert(receiver->sending != NULL);
			assert(sender->process_flags == SR_STATUS_SENDING);
			assert(sender->msg != 0);
			assert(sender->recvfrom == SR_TARGET_NONE);
			assert(sender->sendto == proc2pid(receiver));
		}
	}
	else if (source >= 0 && source < NUM_TASKS + NUM_PROCS)
	{
		sender = &proc_table[source];
		if ((sender->process_flags & SR_STATUS_SENDING) && (sender->sendto == proc2pid(receiver)))
		{
			can_copy = 1;
			process_t* p = receiver->sending;

			assert(p); // sender must have been appended to the queue, so the queue must not be NULL

			while (p)
			{
				assert(sender->process_flags & SR_STATUS_SENDING);

				if (proc2pid(p) == source) // if p is the one
					break;

				prev = p;
				p = p->next_sending;
			}

			assert(receiver->process_flags == 0);
			assert(receiver->msg == NULL);
			assert(receiver->recvfrom == SR_TARGET_NONE);
			assert(receiver->sendto == SR_TARGET_NONE);
			assert(receiver->sending != NULL);
			assert(sender->process_flags == SR_STATUS_SENDING);
			assert(sender->msg != NULL);
			assert(sender->recvfrom == SR_TARGET_NONE);
			assert(sender->sendto == proc2pid(receiver));
		}
	}

	if (can_copy)
	{
		// It's determined from which proc the message will
		// be copied. Note that this proc must have been
		// waiting for this moment in the queue, so we should
		// remove it from the queue.
		if (sender == receiver->sending) // the 1st one
		{
			assert(prev == NULL);
			receiver->sending = sender->next_sending;
			sender->next_sending = NULL;
		}
		else
		{
			assert(prev);
			prev->next_sending = sender->next_sending;
			sender->next_sending = NULL;
		}

		assert(msg);
		assert(sender->msg);

		memcpy(va2la(proc2pid(receiver), msg), va2la(proc2pid(sender), sender->msg), sizeof(message_t));

		sender->msg = 0;
		sender->sendto = SR_TARGET_NONE;
		sender->process_flags &= ~SR_STATUS_SENDING;

		process_unblock(sender);
	}
	else
	{
		// nobody's sending any msg
		// Set process_flags so that receiver will not be scheduled until it is unblocked.
		receiver->process_flags |= SR_STATUS_RECEIVING;

		receiver->msg = (message_t*)msg;
		receiver->recvfrom = source;
		process_block(receiver);

		assert(receiver->process_flags == SR_STATUS_RECEIVING);
		assert(receiver->msg != 0);
		assert(receiver->recvfrom != SR_TARGET_NONE);
		assert(receiver->sendto == SR_TARGET_NONE);
		assert(receiver->has_int_msg == false);
	}

	return 0;
}