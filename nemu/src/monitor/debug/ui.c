#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);
void isa_reg_display();
void isa_reg_display_reg(char *);

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char *rl_gets()
{
  static char *line_read = NULL;

  if (line_read)
  {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read)
  {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args)
{
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args)
{
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args)
{
  int num_steps = 1;
  if (args != NULL)
  {
    num_steps = atoi(args);
  }
  Log("%d", num_steps);
  if (num_steps == 0)
  {
    cpu_exec(1);
  }
  else
  {
    cpu_exec(num_steps);
  }

  return 0;
}

static int cmd_new_wp(char *args)
{
  if (!new_wp(args))
  {
    Log("cmd_setwp: 监视点创建失败\n");
    return -1;
  }
  return 0;
}
// 移除监视点
static int cmd_rm_wp(char *args)
{
  rm_wp(strtol(args, NULL, 10));
  return 0;
}

static int cmd_expr(char *args)
{
  bool success = true;
  uint8_t res = expr(args, &success);
  printf("result is %u\n", res);
  return 0;
}

static int cmd_info(char *args)
{
  if (args == NULL)
  {
    printf("usage:\n \
    info w: infos about watchpoint\n\
    info r: infos about registers\n\
    info r xxx: infos about certain register\n\
    ");
  }
  printf("args: %s\n", args);
  if (strcasecmp(args, "r") == 0)
  {
    isa_reg_display();
  }
  else if (strcasecmp(args, "w") == 0)
  {
    print_wps();
  }
  else if (args[0] == 'r' && args[1] == ' ')
  {
    char *reg_name = args + 2;
    while (reg_name != NULL && reg_name == ' ')
    {
      reg_name += 1;
    }
  printf("args: %s\n", reg_name);
    isa_reg_display_reg(reg_name);
  }
  else
  {
    printf("info unknown, TODO\n");
  }

  return 0;
}

static struct
{
  char *name;
  char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display informations about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},
    {"si", "Single step the program", cmd_si},
    {"info", "info about the monitor", cmd_info},
    {"w", "new watchpoint", cmd_new_wp},
    {"d", "remove watchpoint", cmd_rm_wp},
    {"p", "Evaluate given expression", cmd_expr},
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args)
{
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL)
  {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++)
    {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else
  {
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(arg, cmd_table[i].name) == 0)
      {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void ui_mainloop(int is_batch_mode)
{
  if (is_batch_mode)
  {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;)
  {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL)
    {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end)
    {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(cmd, cmd_table[i].name) == 0)
      {
        if (cmd_table[i].handler(args) < 0)
        {
          return;
        }
        break;
      }
    }

    if (i == NR_CMD)
    {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}
