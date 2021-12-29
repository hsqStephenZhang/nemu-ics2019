int init_monitor(int, char *[]);
void ui_mainloop(int);

#define DEBUG_EXPR

int main(int argc, char *argv[])
{
#ifdef DEBUG_EXPR
  void debug_expression();
  void init_regex();
  init_regex();
  debug_expression();
#else
  /* Initialize the monitor. */
  int is_batch_mode = init_monitor(argc, argv);

  /* Receive commands from user. */
  ui_mainloop(is_batch_mode);
#endif

  return 0;
}

#ifdef DEBUG_EXPR
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int expr(char *e, char *success);

void debug_expression()
{
  FILE *fp = fopen("/home/zc/ics2019/nemu/tools/gen-expr/input", "r");
  if (fp == NULL)
  {
    printf("read file failed\n");
    exit(-1);
  }
  else
  {
    char *line = NULL;
    size_t read;
    size_t len;
    int num_lines = 0;
    printf("read file success\n");
    while ((read = getline(&line, &len, fp)) != -1)
    {
      int val;
      char buffer[1024 * 1024] = {0};
      sscanf(line, "%u %[^\n]", &val, buffer);
      char flag;
      unsigned ans = expr(buffer, &flag);
      printf("result: %u, eval: %u\n", val, ans);
      num_lines++;
    }
    printf("num lines:%d\n", num_lines);

    if (line)
    {
      free(line);
    }

    fclose(fp);
  }
}

#endif
