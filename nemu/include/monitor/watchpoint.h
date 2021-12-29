#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint
{
  int NO;
  struct watchpoint *next;

  char *expr;
  uint32_t prev_value;
  bool just_init;
} WP;

// print all watchpoints
void print_wps();

// allocate one new watchpoint
WP *new_wp(char *expr);

// free one watchpoint
void rm_wp(int NO);

// check watchpoint
bool check_wps();

#endif
