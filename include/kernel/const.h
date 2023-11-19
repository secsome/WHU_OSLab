#pragma once

#define EXTERN extern

#define FORCEINLINE __attribute__((always_inline))

#define NULL (0)

/* 函数类型 */
#define	PUBLIC
#define	PRIVATE	static
#define NAKED __attribute__((naked))

/* GDT 和 IDT 中描述符的个数 */
#define	GDT_SIZE	128
#define	IDT_SIZE	256

/* 权限 */
#define	PRIVILEGE_KRNL	0
#define	PRIVILEGE_TASK	1
#define	PRIVILEGE_USER	3


