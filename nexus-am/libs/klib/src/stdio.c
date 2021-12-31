#include "klib.h"
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static void putc(char *dst, char ch, int offset)
{
  if (dst)
  {
    dst[offset] = ch;
  }
  else
  {
    _putc(ch);
  }
}

static int int_num_char(long value, int base)
{
  int count = 1;
  while (value / base)
  {
    count++;
    value /= base;
  }
  return count;
}

static int print_s(const char *data, char *dst)
{
  const char *count = data;
  while (*count)
  {
    putc(dst, *count, count - data);
    count++;
  }
  return count - data;
}

static char itoc(int value)
{
  if (value < 10)
  {
    return value + '0';
  }
  else
  {
    return value - 10 + 'a';
  }
}

static int print_d(long d, int count, char *dst, int base)
{
  assert(base == 10 || base == 16);
  if (d < 0)
  {
    putc(dst, '-', count);
    return print_d(-d, count, dst ? dst + 1 : dst, base) + 1;
  }

  if (d / base)
  {
    count += print_d(d / base, count, dst, base);
    putc(dst, itoc(d % base), count);
  }
  else
  {
    putc(dst, itoc(d), count);
  }
  return count + 1;
}

static int print_p(unsigned long p, int count, char *dst)
{
  unsigned int base = 16;
  assert(p >= 0);
  if (p / base)
  {
    count += print_p(p / base, count, dst);
    putc(dst, itoc(p % base), count);
  }
  else
  {
    putc(dst, itoc(p), count);
  }
  return count + 1;
}

int printf(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  int count = vsprintf(NULL, fmt, ap);
  va_end(ap);
  putc(NULL, '\0', count);
  return count;
}

int vsprintf(char *out, const char *fmt, va_list ap)
{

  char *s;
  long d;
  char ch;
  int count = 0;
  int width;
  while ((ch = *fmt++))
  {
    if (ch != '%')
    {
      putc(out, ch, count);
      count++;
      continue;
    }
    switch (*fmt++)
    {
    case 's':
      s = va_arg(ap, char *);
      count += out ? print_s(s, out + count) : print_s(s, NULL);
      break;
    case 'd':
      d = va_arg(ap, int);
      count += out ? print_d(d, 0, out + count, 10) : print_d(d, 0, NULL, 10);
      break;
    case 'c':
      ch = va_arg(ap, int);
      putc(out, ch, count);
      count++;
      break;
    case 'x':
      d = va_arg(ap, int);
      count += out ? print_p(d, 0, out + count) : print_p(d, 0, NULL);
      break;
    case 'p':
      d = va_arg(ap, unsigned long);
      putc(out, '0', count++);
      putc(out, 'x', count++);
      count += print_p(d, 0, out ? out + count : out);
      break;
    case '0':
      width = *fmt++ - '0';
      d = va_arg(ap, int);
      for (int i = 0; i < width - int_num_char(d, *fmt == 'd' ? 10 : 16); i++)
      {
        putc(out, '0', count + i);
      }
      assert(width == 8);
      if (*fmt == 'd')
      {
        out ? print_d(d, 0, out + count, 10) : print_d(d, 0, NULL, 10);
        count += width;
      }
      else if (*fmt == 'x')
      {
        out ? print_d(d, 0, out + count, 16) : print_d(d, 0, NULL, 16);
        count += width;
      }
      else
      {
        // printf("unexpected format %c", ch);
        assert(0);
      }
      fmt++;
    default:
      printf("unexpected format %d", ch);
      assert(0);
    }
  }
  return count;
}

int sprintf(char *out, const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  int count = vsprintf(out, fmt, ap);
  va_end(ap);
  putc(out, '\0', count);
  return count;
}

#endif
