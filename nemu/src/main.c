int init_monitor(int, char *[]);
void ui_mainloop(int);

#define DEBUG_EXPR

int main(int argc, char *argv[])
{
#ifdef DEBUG_EXPR
  void debug_expression();
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
      sscanf(line, "%u %[^\n]", &val, &buffer);
      printf("result: %u, expression: %s\n", val, buffer);
      num_lines++;
    }
    printf("num lines:%d\n", num_lines);

    if (line)
    {
      free(line);
    }

    fclose(fp);
  }

  // #define NUM_LINES 100

  //   FILE *fp = fopen("tools/gen-expr/input", "r");
  //   char expression[70000];
  //   char *str;
  //   for (int i = 0; i < NUM_LINES; i++)
  //   {
  //     char flag = 1;

  //     expression[0] = '\0';
  //     fgets(expression, 70000, fp);
  //     unsigned answer = 0, ans;
  //     str = strtok(expression, " ");
  //     sscanf(expression, "%u", &answer);
  //     str = expression + strlen(expression) + 1;
  //     printf("str:%s\n",str);
  //     ans = expr(str, &flag);
  //     if (answer != ans)
  //       printf("%d Wrong\n", i);
  //     else
  //       printf("correct %d\n", i);

  //   }
}

#endif // DEBUG
