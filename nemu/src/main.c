int init_monitor(int, char *[]);
void ui_mainloop(int);

#define DEBUG_EXPR

int main(int argc, char *argv[])
{
#ifdef DEBUG_EXPR
  void debug_expression();
  debug_expression();
#endif

  /* Initialize the monitor. */
  int is_batch_mode = init_monitor(argc, argv);

  /* Receive commands from user. */
  ui_mainloop(is_batch_mode);

  return 0;
}

#ifdef DEBUG_EXPR
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

void debug_expression()
{
  // char a[]="123    123U";
  // unsigned int val;
  // char *s = 0;
  // scanf("%d%s", a, &val,s);
  FILE *fp = fopen("/home/zc/ics2019/nemu/tools/gen-expr/input", "r");
  if (fp == NULL)
  {
    printf("read file failed\n");
    exit(-1);
  }
  else
  {
    char line[65536];
    size_t read;
    size_t len;
    printf("read file success\n");
    while ((read = getline(line, &len, fp)) != -1) {
        printf("Retrieved line of length %d:\n", read);
        printf("%s", line);
    }

    fclose(fp);
  }
}

#endif // DEBUG
