#include "common.h"
#include "syscall.h"

static int __brk;

int do_open(const char *path, int flags, int mode)
{
  return fs_open(path, flags, mode);
}

int do_close(int fd)
{
  return fs_close(fd);
}

int do_read(int fd, void *buf, size_t count)
{
  // Log("fd: %d, count:%d", fd, count);
  if (fd >= 0 && fd <= 2)
  {
    return 0;
  }
  return fs_read(fd, buf, count);
}

int do_write(int fd, const void *buf, size_t count)
{
  // Log("fd: %d, count:%d", fd, count);
  return fs_write(fd, buf, count);
}

size_t do_lseek(int fd, size_t offset, int whence)
{
  return fs_lseek(fd, offset, whence);
}

int do_brk(int addr)
{
  __brk = addr;
  return 0;
}

_Context *do_syscall(_Context *c)
{
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  switch (a[0])
  {
  case SYS_exit:
    _halt(0);
    break;
  case SYS_yield:
    _yield();
    c->GPRx = 0;
    break;
  case SYS_open:
    c->GPRx = fs_open((const char *)(a[1]), a[2], a[3]);
    break;
  case SYS_read:
    c->GPRx = do_read(a[1], (void *)(a[2]), a[3]);
    break;
  case SYS_write:
    c->GPRx = do_write(a[1], (void *)(a[2]), a[3]);
    break;
  case SYS_kill:
  case SYS_getpid:
    break;
  case SYS_close:
    c->GPRx = do_close(a[1]);
    break;
  case SYS_lseek:
    c->GPRx = do_lseek(a[1], a[2], a[3]);
    break;
  case SYS_brk:
    c->GPRx = do_brk(a[1]);
    break;
  case SYS_fstat:
  case SYS_time:
  case SYS_signal:
    break;
  case SYS_execve:
    printf("%s\n", a[1]);
    void naive_uload(int, const char *);
    naive_uload(NULL, (const char *)a[1]);
    c->GPR2 = SYS_exit;
    do_syscall(c);
    break;
  case SYS_fork:
  case SYS_link:
  case SYS_unlink:
  case SYS_wait:
  case SYS_times:
  case SYS_gettimeofday:
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
