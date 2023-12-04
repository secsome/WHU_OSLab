#include <sys/const.h>
#include <lib/printf.h>
#include <lib/float.h>
#include <lib/puts.h>
#include <lib/stdint.h>
#include <lib/stddef.h>
#include <lib/syscall.h>

HEADER_CPP_BEGIN

// 'ntoa' conversion buffer size, this must be big enough to hold one converted
// numeric number including padded zeros (dynamically created on stack)
static constexpr size_t PRINTF_NTOA_BUFFER_SIZE = 32u;

// 'ftoa' conversion buffer size, this must be big enough to hold one converted
// float number including padded zeros (dynamically created on stack)
static constexpr size_t PRINTF_FTOA_BUFFER_SIZE = 32u;

// define the default floating point precision
static constexpr size_t PRINTF_DEFAULT_FLOAT_PRECISION = 6;

// define the largest float suitable to print with %f
static constexpr double PRINTF_MAX_FLOAT = 1e9;

enum
{
    FLAGS_ZEROPAD = 1 << 0,
    FLAGS_LEFT = 1 << 1,
    FLAGS_PLUS = 1 << 2,
    FLAGS_SPACE = 1 << 3,
    FLAGS_HASH = 1 << 4,
    FLAGS_UPPERCASE = 1 << 5,
    FLAGS_CHAR = 1 << 6,
    FLAGS_SHORT = 1 << 7,
    FLAGS_LONG = 1 << 8,
    FLAGS_LONG_LONG = 1 << 9,
    FLAGS_PRECISION = 1 << 10,
    FLAGS_ADAPT_EXP = 1 << 11,
};

using out_fct_t = void(char character, void *buffer, size_t idx, size_t maxlen);

// internal buffer output
static constexpr void out_buffer_internal(char character, void *buffer, size_t idx, size_t maxlen)
{
    if (idx < maxlen)
        reinterpret_cast<char *>(buffer)[idx] = character;
}

// internal null output
static constexpr void out_null_internal(char character, void *buffer, size_t idx, size_t maxlen)
{
    // UNREFERENCED_PARAMETER(character);
    // UNREFERENCED_PARAMETER(buffer);
    // UNREFERENCED_PARAMETER(idx);
    // UNREFERENCED_PARAMETER(maxlen);
}

// internal putchar wrapper
static constexpr void out_char_internal(char character, void *buffer, size_t idx, size_t maxlen)
{
    // UNREFERENCED_PARAMETER(buffer);
    // UNREFERENCED_PARAMETER(idx);
    // UNREFERENCED_PARAMETER(maxlen);
    if (character)
        putchar(character);
}

// internal secure strlen
static constexpr size_t strnlen_internal(const char *s, size_t maxsize)
{
    size_t length = 0;
    while (*s++ && maxsize--)
        ++length;
    return length;
}

// output the specified string in reverse, taking care of any zero-padding
static constexpr size_t out_rev_internal(out_fct_t out, char *buffer, size_t idx, size_t maxlen, const char *buf, size_t len, unsigned int width, unsigned int flags)
{
    const size_t start_idx = idx;
    // pad spaces up to given width
    if (!(flags & FLAGS_LEFT) && !(flags & FLAGS_ZEROPAD))
    {
        for (size_t i = len; i < width; ++i)
            out(' ', buffer, idx++, maxlen);
    }

    // reverse string
    while (len)
        out(buf[--len], buffer, idx++, maxlen);

    // append pad spaces up to given width
    if (flags & FLAGS_LEFT)
    {
        while (idx - start_idx < width)
            out(' ', buffer, idx++, maxlen);
    }

    return idx;
}

// internal itoa format
static constexpr size_t ntoa_format_internal(out_fct_t out, char *buffer, size_t idx, size_t maxlen, char *buf, size_t len, bool negative, unsigned int base, unsigned int prec, unsigned int width, unsigned int flags)
{
    // pad leading zeros
    if (!(flags & FLAGS_LEFT))
    {
        if (width && (flags & FLAGS_ZEROPAD) && (negative || (flags & (FLAGS_PLUS | FLAGS_SPACE))))
            width--;
        while ((len < prec) && (len < PRINTF_NTOA_BUFFER_SIZE))
            buf[len++] = '0';
        while ((flags & FLAGS_ZEROPAD) && (len < width) && (len < PRINTF_NTOA_BUFFER_SIZE))
            buf[len++] = '0';
    }

    // handle hash
    if (flags & FLAGS_HASH)
    {
        if (!(flags & FLAGS_PRECISION) && len && ((len == prec) || (len == width)))
        {
            len--;
            if (len && (base == 16U))
                len--;
        }
        if ((base == 16U) && !(flags & FLAGS_UPPERCASE) && (len < PRINTF_NTOA_BUFFER_SIZE))
            buf[len++] = 'x';
        else if ((base == 16U) && (flags & FLAGS_UPPERCASE) && (len < PRINTF_NTOA_BUFFER_SIZE))
            buf[len++] = 'X';
        else if ((base == 2U) && (len < PRINTF_NTOA_BUFFER_SIZE))
            buf[len++] = 'b';
        if (len < PRINTF_NTOA_BUFFER_SIZE)
            buf[len++] = '0';
    }

    if (len < PRINTF_NTOA_BUFFER_SIZE)
    {
        if (negative)
            buf[len++] = '-';
        else if (flags & FLAGS_PLUS)
            buf[len++] = '+'; // ignore the space if the '+' exists
        else if (flags & FLAGS_SPACE)
            buf[len++] = ' ';
    }
    return out_rev_internal(out, buffer, idx, maxlen, buf, len, width, flags);
}

// internal itoa for 'long' type
static constexpr size_t ntoa_l_internal(out_fct_t out, char *buffer, size_t idx, size_t maxlen, unsigned long value, bool negative, unsigned long base, unsigned int prec, unsigned int width, unsigned int flags)
{
    char buf[PRINTF_NTOA_BUFFER_SIZE];
    size_t len = 0U;

    // no hash for 0 values
    if (!value)
        flags &= ~FLAGS_HASH;

    // write if precision != 0 and value is != 0
    if (!(flags & FLAGS_PRECISION) || value)
    {
        do
        {
            const char digit = (char)(value % base);
            buf[len++] = digit < 10 ? '0' + digit : (flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;
            value /= base;
        } while (value && (len < PRINTF_NTOA_BUFFER_SIZE));
    }
    return ntoa_format_internal(out, buffer, idx, maxlen, buf, len, negative, (unsigned int)base, prec, width, flags);
}

static constexpr size_t ntoa_ll_internal(out_fct_t out, char *buffer, size_t idx, size_t maxlen, unsigned long long value, bool negative, unsigned long long base, unsigned int prec, unsigned int width, unsigned int flags)
{
    char buf[PRINTF_NTOA_BUFFER_SIZE];
    size_t len = 0U;

    // no hash for 0 values
    if (!value)
        flags &= ~FLAGS_HASH;

    // write if precision != 0 and value is != 0
    if (!(flags & FLAGS_PRECISION) || value)
    {
        do
        {
            const char digit = (char)(value % base);
            buf[len++] = digit < 10 ? '0' + digit : (flags & FLAGS_UPPERCASE ? 'A' : 'a') + digit - 10;
            value /= base;
        } while (value && (len < PRINTF_NTOA_BUFFER_SIZE));
    }

    return ntoa_format_internal(out, buffer, idx, maxlen, buf, len, negative, (unsigned int)base, prec, width, flags);
}

// forward declaration so that ftoa_internal can switch to exp notation for values > PRINTF_MAX_FLOAT
static constexpr size_t etoa_internal(out_fct_t out, char *buffer, size_t idx, size_t maxlen, double value, unsigned int prec, unsigned int width, unsigned int flags);

// internal ftoa for fixed decimal floating point
static constexpr size_t ftoa_internal(out_fct_t out, char *buffer, size_t idx, size_t maxlen, double value, unsigned int prec, unsigned int width, unsigned int flags)
{
    char buf[PRINTF_FTOA_BUFFER_SIZE];
    size_t len = 0U;
    double diff = 0.0;

    // powers of 10
    constexpr double pow10[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};

    // test for special values
    if (value != value)
        return out_rev_internal(out, buffer, idx, maxlen, "nan", 3, width, flags);
    if (value < -DBL_MAX)
        return out_rev_internal(out, buffer, idx, maxlen, "fni-", 4, width, flags);
    if (value > DBL_MAX)
        return out_rev_internal(out, buffer, idx, maxlen, (flags & FLAGS_PLUS) ? "fni+" : "fni", (flags & FLAGS_PLUS) ? 4U : 3U, width, flags);

    // test for very large values
    // print them using exp mode
    if ((value > PRINTF_MAX_FLOAT) || (value < -PRINTF_MAX_FLOAT))
        return etoa_internal(out, buffer, idx, maxlen, value, prec, width, flags);

    // test for negative
    bool negative = false;
    if (value < 0)
    {
        negative = true;
        value = 0 - value;
    }

    // set default precision, if not set explicitly
    if (!(flags & FLAGS_PRECISION))
        prec = PRINTF_DEFAULT_FLOAT_PRECISION;

    // limit precision to 9, cause a prec >= 10 can lead to overflow errors
    while ((len < PRINTF_FTOA_BUFFER_SIZE) && (prec > 9U))
    {
        buf[len++] = '0';
        prec--;
    }

    int whole = (int)value;
    double tmp = (value - whole) * pow10[prec];
    unsigned long frac = (unsigned long)tmp;
    diff = tmp - frac;

    if (diff > 0.5)
    {
        ++frac;
        // handle rollover, e.g. case 0.99 with prec 1 is 1.0
        if (frac >= pow10[prec])
        {
            frac = 0;
            ++whole;
        }
    }
    else if (diff < 0.5)
    {
    }
    else if ((frac == 0U) || (frac & 1U))
    {
        // if halfway, round up if odd OR if last digit is 0
        ++frac;
    }

    if (prec == 0U)
    {
        diff = value - (double)whole;
        if ((!(diff < 0.5) || (diff > 0.5)) && (whole & 1))
        {
            // exactly 0.5 and ODD, then round up
            // 1.5 -> 2, but 2.5 -> 2
            ++whole;
        }
    }
    else
    {
        unsigned int count = prec;
        // now do fractional part, as an unsigned number
        while (len < PRINTF_FTOA_BUFFER_SIZE)
        {
            --count;
            buf[len++] = (char)(48U + (frac % 10U));
            if (!(frac /= 10U))
                break;
        }
        while ((len < PRINTF_FTOA_BUFFER_SIZE) && (count-- > 0U)) // add extra 0s
            buf[len++] = '0';
        if (len < PRINTF_FTOA_BUFFER_SIZE) // add decimal
            buf[len++] = '.';
    }

    // do whole part, number is reversed
    while (len < PRINTF_FTOA_BUFFER_SIZE)
    {
        buf[len++] = (char)(48 + (whole % 10));
        if (!(whole /= 10))
            break;
    }

    // pad leading zeros
    if (!(flags & FLAGS_LEFT) && (flags & FLAGS_ZEROPAD))
    {
        if (width && (negative || (flags & (FLAGS_PLUS | FLAGS_SPACE))))
            width--;
        while ((len < width) && (len < PRINTF_FTOA_BUFFER_SIZE))
            buf[len++] = '0';
    }

    if (len < PRINTF_FTOA_BUFFER_SIZE)
    {
        if (negative)
            buf[len++] = '-';
        else if (flags & FLAGS_PLUS)
            buf[len++] = '+'; // ignore the space if the '+' exists
        else if (flags & FLAGS_SPACE)
            buf[len++] = ' ';
    }

    return out_rev_internal(out, buffer, idx, maxlen, buf, len, width, flags);
}

// internal ftoa variant for exponential floating-point type
static constexpr size_t etoa_internal(out_fct_t out, char *buffer, size_t idx, size_t maxlen, double value, unsigned int prec, unsigned int width, unsigned int flags)
{
    // check for NaN and special values
    if ((value != value) || (value > DBL_MAX) || (value < -DBL_MAX))
        return ftoa_internal(out, buffer, idx, maxlen, value, prec, width, flags);

    // determine the sign
    const bool negative = value < 0;
    if (negative)
        value = -value;

    // default precision
    if (!(flags & FLAGS_PRECISION))
        prec = PRINTF_DEFAULT_FLOAT_PRECISION;

    // determine the decimal exponent
    // based on the algorithm by David Gay (https://www.ampl.com/netlib/fp/dtoa.c)
    union
    {
        uint64_t U;
        double F;
    } conv;

    conv.F = value;
    int exp2 = (int)((conv.U >> 52U) & 0x07FFU) - 1023;          // effectively log2
    conv.U = (conv.U & ((1ULL << 52U) - 1U)) | (1023ULL << 52U); // drop the exponent so conv.F is now in [1,2)
    // now approximate log10 from the log2 integer part and an expansion of ln around 1.5
    int expval = (int)(0.1760912590558 + exp2 * 0.301029995663981 + (conv.F - 1.5) * 0.289529654602168);
    // now we want to compute 10^expval but we want to be sure it won't overflow
    exp2 = (int)(expval * 3.321928094887362 + 0.5);
    const double z = expval * 2.302585092994046 - exp2 * 0.6931471805599453;
    const double z2 = z * z;
    conv.U = (uint64_t)(exp2 + 1023) << 52U;
    // compute exp(z) using continued fractions, see https://en.wikipedia.org/wiki/Exponential_function#Continued_fractions_for_ex
    conv.F *= 1 + 2 * z / (2 - z + (z2 / (6 + (z2 / (10 + z2 / 14)))));
    // correct for rounding errors
    if (value < conv.F)
    {
        expval--;
        conv.F /= 10;
    }

    // the exponent format is "%+03d" and largest value is "307", so set aside 4-5 characters
    unsigned int minwidth = ((expval < 100) && (expval > -100)) ? 4U : 5U;

    // in "%g" mode, "prec" is the number of *significant figures* not decimals
    if (flags & FLAGS_ADAPT_EXP)
    {
        // do we want to fall-back to "%f" mode?
        if ((value >= 1e-4) && (value < 1e6))
        {
            if ((int)prec > expval)
                prec = (unsigned)((int)prec - expval - 1);
            else
                prec = 0;
            flags |= FLAGS_PRECISION; // make sure ftoa_internal respects precision
            // no characters in exponent
            minwidth = 0U;
            expval = 0;
        }
        else
        {
            // we use one sigfig for the whole part
            if ((prec > 0) && (flags & FLAGS_PRECISION))
                --prec;
        }
    }

    // will everything fit?
    unsigned int fwidth = width;
    if (width > minwidth) // we didn't fall-back so subtract the characters required for the exponent
        fwidth -= minwidth;
    else // not enough characters, so go back to default sizing
        fwidth = 0U;
    if ((flags & FLAGS_LEFT) && minwidth) // if we're padding on the right, DON'T pad the floating part
        fwidth = 0U;

    // rescale the float value
    if (expval)
        value /= conv.F;

    // output the floating part
    const size_t start_idx = idx;
    idx = ftoa_internal(out, buffer, idx, maxlen, negative ? -value : value, prec, fwidth, flags & ~FLAGS_ADAPT_EXP);

    // output the exponent part
    if (minwidth)
    {
        // output the exponential symbol
        out((flags & FLAGS_UPPERCASE) ? 'E' : 'e', buffer, idx++, maxlen);
        // output the exponent value
        idx = ntoa_l_internal(out, buffer, idx, maxlen, (expval < 0) ? -expval : expval, expval < 0, 10, 0, minwidth - 1, FLAGS_ZEROPAD | FLAGS_PLUS);
        // might need to right-pad spaces
        if (flags & FLAGS_LEFT)
        {
            while (idx - start_idx < width)
                out(' ', buffer, idx++, maxlen);
        }
    }
    return idx;
}

// internal isdigit
static constexpr bool isdigit_internal(char c)
{
    return c >= '0' && c <= '9';
}

static constexpr unsigned int atoi_internal(const char **s)
{
    unsigned int i = 0U;
    while (isdigit_internal(**s))
        i = i * 10U + (unsigned int)(*((*s)++) - '0');
    return i;
}

// internal vsnprintf
static constexpr int vsnprintf_internal(out_fct_t out, char *buffer, const size_t maxlen, const char *format, va_list va)
{
    unsigned int flags, width, precision, n;
    size_t idx = 0U;

    if (!buffer) // use null output function
        out = out_null_internal;

    while (*format)
    {
        // format specifier?  %[flags][width][.precision][length]
        if (*format != '%')
        {
            // no
            out(*format, buffer, idx++, maxlen);
            format++;
            continue;
        }
        else // yes, evaluate it
            format++;

        // evaluate flags
        flags = 0U;
        do
        {
            switch (*format)
            {
            case '0':
                flags |= FLAGS_ZEROPAD;
                format++;
                n = 1U;
                break;
            case '-':
                flags |= FLAGS_LEFT;
                format++;
                n = 1U;
                break;
            case '+':
                flags |= FLAGS_PLUS;
                format++;
                n = 1U;
                break;
            case ' ':
                flags |= FLAGS_SPACE;
                format++;
                n = 1U;
                break;
            case '#':
                flags |= FLAGS_HASH;
                format++;
                n = 1U;
                break;
            default:
                n = 0U;
                break;
            }
        } while (n);

        // evaluate width field
        width = 0U;
        if (isdigit_internal(*format))
            width = atoi_internal(&format);
        else if (*format == '*')
        {
            const int w = va_arg(va, int);
            if (w < 0)
            {
                flags |= FLAGS_LEFT; // reverse padding
                width = (unsigned int)-w;
            }
            else
                width = (unsigned int)w;
            format++;
        }

        // evaluate precision field
        precision = 0U;
        if (*format == '.')
        {
            flags |= FLAGS_PRECISION;
            format++;
            if (isdigit_internal(*format))
                precision = atoi_internal(&format);
            else if (*format == '*')
            {
                const int prec = (int)va_arg(va, int);
                precision = prec > 0 ? (unsigned int)prec : 0U;
                format++;
            }
        }

        // evaluate length field
        switch (*format)
        {
        case 'l':
            flags |= FLAGS_LONG;
            format++;
            if (*format == 'l')
            {
                flags |= FLAGS_LONG_LONG;
                format++;
            }
            break;
        case 'h':
            flags |= FLAGS_SHORT;
            format++;
            if (*format == 'h')
            {
                flags |= FLAGS_CHAR;
                format++;
            }
            break;
        case 't':
            flags |= (sizeof(ptrdiff_t) == sizeof(long) ? FLAGS_LONG : FLAGS_LONG_LONG);
            format++;
            break;
        case 'j':
            flags |= (sizeof(intmax_t) == sizeof(long) ? FLAGS_LONG : FLAGS_LONG_LONG);
            format++;
            break;
        case 'z':
            flags |= (sizeof(size_t) == sizeof(long) ? FLAGS_LONG : FLAGS_LONG_LONG);
            format++;
            break;
        default:
            break;
        }

        // evaluate specifier
        switch (*format)
        {
        case 'd':
        case 'i':
        case 'u':
        case 'x':
        case 'X':
        case 'o':
        case 'b':
        {
            // set the base
            unsigned int base;
            if (*format == 'x' || *format == 'X')
                base = 16U;
            else if (*format == 'o')
                base = 8U;
            else if (*format == 'b')
                base = 2U;
            else
            {
                base = 10U;
                flags &= ~FLAGS_HASH; // no hash for dec format
            }
            // uppercase
            if (*format == 'X')
                flags |= FLAGS_UPPERCASE;

            // no plus or space flag for u, x, X, o, b
            if ((*format != 'i') && (*format != 'd'))
                flags &= ~(FLAGS_PLUS | FLAGS_SPACE);

            // ignore '0' flag when precision is given
            if (flags & FLAGS_PRECISION)
                flags &= ~FLAGS_ZEROPAD;

            // convert the integer
            if ((*format == 'i') || (*format == 'd'))
            {
                // signed
                if (flags & FLAGS_LONG_LONG)
                {
                    const long long value = va_arg(va, long long);
                    idx = ntoa_ll_internal(out, buffer, idx, maxlen, (unsigned long long)(value > 0 ? value : 0 - value), value < 0, base, precision, width, flags);
                }
                else if (flags & FLAGS_LONG)
                {
                    const long value = va_arg(va, long);
                    idx = ntoa_l_internal(out, buffer, idx, maxlen, (unsigned long)(value > 0 ? value : 0 - value), value < 0, base, precision, width, flags);
                }
                else
                {
                    const int value = (flags & FLAGS_CHAR) ? (char)va_arg(va, int) : (flags & FLAGS_SHORT) ? (short int)va_arg(va, int)
                                                                                                           : va_arg(va, int);
                    idx = ntoa_l_internal(out, buffer, idx, maxlen, (unsigned int)(value > 0 ? value : 0 - value), value < 0, base, precision, width, flags);
                }
            }
            else
            {
                // unsigned
                if (flags & FLAGS_LONG_LONG)
                    idx = ntoa_ll_internal(out, buffer, idx, maxlen, va_arg(va, unsigned long long), false, base, precision, width, flags);
                else if (flags & FLAGS_LONG)
                    idx = ntoa_l_internal(out, buffer, idx, maxlen, va_arg(va, unsigned long), false, base, precision, width, flags);
                else
                {
                    const unsigned int value = (flags & FLAGS_CHAR) ? (unsigned char)va_arg(va, unsigned int) : (flags & FLAGS_SHORT) ? (unsigned short int)va_arg(va, unsigned int)
                                                                                                                                      : va_arg(va, unsigned int);
                    idx = ntoa_l_internal(out, buffer, idx, maxlen, value, false, base, precision, width, flags);
                }
            }
            format++;
            break;
        }
        case 'f':
        case 'F':
            if (*format == 'F')
                flags |= FLAGS_UPPERCASE;
            idx = ftoa_internal(out, buffer, idx, maxlen, va_arg(va, double), precision, width, flags);
            format++;
            break;
        case 'e':
        case 'E':
        case 'g':
        case 'G':
            if ((*format == 'g') || (*format == 'G'))
                flags |= FLAGS_ADAPT_EXP;
            if ((*format == 'E') || (*format == 'G'))
                flags |= FLAGS_UPPERCASE;
            idx = etoa_internal(out, buffer, idx, maxlen, va_arg(va, double), precision, width, flags);
            format++;
            break;
        case 'c':
        {
            unsigned int l = 1U;
            // pre padding
            if (!(flags & FLAGS_LEFT))
            {
                while (l++ < width)
                    out(' ', buffer, idx++, maxlen);
            }
            // char output
            out((char)va_arg(va, int), buffer, idx++, maxlen);
            // post padding
            if (flags & FLAGS_LEFT)
            {
                while (l++ < width)
                    out(' ', buffer, idx++, maxlen);
            }
            format++;
            break;
        }

        case 's':
        {
            const char *p = va_arg(va, char *);
            unsigned int l = strnlen_internal(p, precision ? precision : (size_t)-1);
            // pre padding
            if (flags & FLAGS_PRECISION)
                l = (l < precision ? l : precision);
            if (!(flags & FLAGS_LEFT))
            {
                while (l++ < width)
                    out(' ', buffer, idx++, maxlen);
            }
            // string output
            while ((*p != 0) && (!(flags & FLAGS_PRECISION) || precision--))
                out(*(p++), buffer, idx++, maxlen);
            // post padding
            if (flags & FLAGS_LEFT)
            {
                while (l++ < width)
                    out(' ', buffer, idx++, maxlen);
            }
            format++;
            break;
        }

        case 'p':
        {
            width = sizeof(void *) * 2U;
            flags |= FLAGS_ZEROPAD | FLAGS_UPPERCASE;
            const bool is_ll = sizeof(uintptr_t) == sizeof(long long);
            if (is_ll)
                idx = ntoa_ll_internal(out, buffer, idx, maxlen, (uintptr_t)va_arg(va, void *), false, 16U, precision, width, flags);
            else
                idx = ntoa_l_internal(out, buffer, idx, maxlen, (unsigned long)((uintptr_t)va_arg(va, void *)), false, 16U, precision, width, flags);
            format++;
            break;
        }

        case '%':
            out('%', buffer, idx++, maxlen);
            format++;
            break;

        default:
            out(*format, buffer, idx++, maxlen);
            format++;
            break;
        }
    }
    // termination
    out((char)0, buffer, idx < maxlen ? idx : maxlen - 1U, maxlen);

    // return written chars without terminating \0
    return (int)idx;
}

int printf(const char *format, ...)
{
    va_list va;
    va_start(va, format);
    char buffer[1];
    const int ret = vsnprintf_internal(out_char_internal, buffer, (size_t)-1, format, va);
    va_end(va);
    return ret;
}

int sprintf(char *buffer, const char *format, ...)
{
    va_list va;
    va_start(va, format);
    const int ret = vsnprintf_internal(out_buffer_internal, buffer, (size_t)-1, format, va);
    va_end(va);
    return ret;
}

int snprintf(char *buffer, size_t count, const char *format, ...)
{
    va_list va;
    va_start(va, format);
    const int ret = vsnprintf_internal(out_buffer_internal, buffer, count, format, va);
    va_end(va);
    return ret;
}

int vprintf(const char *format, va_list va)
{
    char buffer[1];
    return vsnprintf_internal(out_char_internal, buffer, (size_t)-1, format, va);
}

int vsnprintf(char *buffer, size_t count, const char *format, va_list va)
{
    return vsnprintf_internal(out_buffer_internal, buffer, count, format, va);
}

static constexpr void printl_out_char_internal(char character, void *buffer, size_t idx, size_t maxlen)
{
    // UNREFERENCED_PARAMETER(buffer);
    // UNREFERENCED_PARAMETER(idx);
    // UNREFERENCED_PARAMETER(maxlen);
    if (character)
    {
        char buffer[2] = { character, 0 };
        lib_writex(buffer);
    }
}
int printl(const char *format, ...)
{
    va_list va;
    va_start(va, format);
    char buffer[1];
    const int ret = vsnprintf_internal(printl_out_char_internal, buffer, (size_t)-1, format, va);
    va_end(va);
    return ret;
}

int vprintl(const char* format, va_list va)
{
    char buffer[1];
    return vsnprintf_internal(printl_out_char_internal, buffer, (size_t)-1, format, va);
}

HEADER_CPP_END