#include "common.h"

extern int _cte_init(_Context *(*handler)(_Event, _Context *));
extern _Context *do_syscall(_Context *c);

static _Context *do_event(_Event e, _Context *c)
{
  switch (e.event)
  {
  case _EVENT_YIELD:
    Log("_EVENT_YIELD");
    break;
  case _EVENT_SYSCALL:
    Log("_EVENT_SYSCALL");
    do_syscall(c);
    break;
  default:
    panic("unimplemented event, ID = %d", e.event);
  }

  return NULL;
}

void init_irq(void)
{
  Log("Initializing interrupt/exception handler...");
  _cte_init(do_event);
}
