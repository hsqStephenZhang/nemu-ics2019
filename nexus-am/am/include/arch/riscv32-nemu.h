#ifndef __ARCH_H__
#define __ARCH_H__

// adjust the order according to trap.S
struct _Context {
  uintptr_t gpr[32], cause, status, epc;
  struct _AddressSpace *as;
};

#define GPR1 gpr[17]
#define GPR2 gpr[0]
#define GPR3 gpr[0]
#define GPR4 gpr[0]
#define GPRx gpr[0]

#endif
