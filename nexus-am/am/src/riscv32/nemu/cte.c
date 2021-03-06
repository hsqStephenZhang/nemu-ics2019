#include <am.h>
#include <riscv32.h>

#define NUM_SYSCALL 20

static _Context *(*user_handler)(_Event, _Context *) = NULL;

_Context *__am_irq_handle(_Context *c)
{
  _Context *next = c;
  if (user_handler)
  {
    _Event ev = {0};
    if (c->cause == -1)
    {
      ev.event = _EVENT_YIELD;
    }
    else if (c->cause >= 0 && c->cause < NUM_SYSCALL)
    {
      ev.event = _EVENT_SYSCALL;
    }
    else
    {
      printf("\33[1;35m[%s,%d,%s] event error, cause: %d \33[0m\n", __FILE__, __LINE__, __func__, c->cause);
      ev.event = _EVENT_ERROR;
    }

    next = user_handler(ev, c);
    if (next == NULL)
    {
      next = c;
    }
  }

  return next;
}

extern void __am_asm_trap(void);

int _cte_init(_Context *(*handler)(_Event, _Context *))
{
  // initialize exception entry
  asm volatile("csrw stvec, %0"
               :
               : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return 0;
}

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg)
{
  return NULL;
}

void _yield()
{
  asm volatile("li a7, -1; ecall");
}

int _intr_read()
{
  return 0;
}

void _intr_write(int enable)
{
}
