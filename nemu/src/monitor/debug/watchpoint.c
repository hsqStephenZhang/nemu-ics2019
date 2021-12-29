#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

WP * new_wp(char * e) {
  bool success = false;
  Log("new watch point is: %s", e);
  uint32_t cur_val = expr(e, &success);
  if (!success) {
    Log("new_wp: create new watchpoint failed, expression invalid\n");
    return NULL;
  }
  // if there is any free watchpoint in the pool, create one
  if (free_) {
    WP * temp = free_;
    free_ = free_->next;
    temp->next = head;
    head = temp;

    head->expr = (char *)malloc(sizeof(char) * (1 + strlen(e)));
    strcpy(head->expr, e);
    head->prev_value = cur_val;
    head->just_init = true;
    return head;
  }
  Log("new_wp: watchpoint pool is full");
  return NULL;
}

void free_wp(WP *wp) {
  if (!head) {
    Log("free watchpoint, there is no watchpoint now, cancelled");
    return;
  }
  WP * temp = head;
  if (head == wp) {
    head = head->next;
    wp->next = free_;
    free_ = wp;
    return;
  }
  WP * next = temp->next;
  while (next && next != wp) {
    temp = temp->next;
    next = next->next;
  }
  if (next == wp) {
    temp->next = next->next;
    next->next = free_;
    free_ = next;
    return;
  }
  Log("free watchpoint, the target watchpoint not exists, cancelled");
  return ;
}

void rm_wp(int NO) {
  if (NO >= 0 && NO < NR_WP) {
    free_wp(&wp_pool[NO]);
    return ;
  }
  Log("remove watchpoint, target watchpoint not exists");
  return ;
}

void print_wps() {
  WP * temp = head;
  while (temp) {
    printf("WP NO.%d \"%s\" value=%d\n", temp->NO, temp->expr, temp->prev_value);
    temp = temp->next;
  }
}

bool check_wps() {
  WP * temp = head;
  bool stop = false;
  bool success = false;
  while(temp) {
    uint32_t cur_val = expr(temp->expr, &success);
    Assert(success, "the expr of watchpoint should be test when creating, so it should be valid now");
    if (success && cur_val != temp->prev_value) {
      temp->prev_value = cur_val;
      stop = true;
      Log("watchpoint No.%d value changed\n", temp->NO);
    }
    temp = temp->next;
  }
  return stop;
}