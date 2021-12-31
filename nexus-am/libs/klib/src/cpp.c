#include "klib.h"

#ifndef __ISA_NATIVE__

void  __attribute__((weak)) __dso_handle() {
}

void  __attribute__((weak)) __cxa_guard_acquire() {
}

void  __attribute__((weak)) __cxa_guard_release() {
}


void  __attribute__((weak)) __cxa_atexit() {
  printf("at exit\n");
  // assert(0);
}

#endif
