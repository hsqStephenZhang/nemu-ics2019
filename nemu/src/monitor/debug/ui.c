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
    Log("cmd_setwp: failed \n");
    return -1;
  }
  return 0;
}

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
    printf("usage:\n\
    info w: infos about watchpoint\n\
    info r: infos about registers\n\
    info r xxx: infos about certain register\n");
    return 0;
  }
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
    while (reg_name != NULL && *reg_name == ' ')
    {
      reg_name += 1;
    }
    if (reg_name != NULL)
    {
      isa_reg_display_reg(reg_name);
    }
    else
    {
      Log("wrong register name");
      isa_reg_display();
    }
  }
  else
  {
    printf("info unknown, TODO\n");
  }

  return 0;
}

static int cmd_scan(char *args)
{
  // `x N EXPR`, e.g. x 10 $esp
  char *N_str = strtok(NULL, " ");

  char *EXPR_str = strtok(NULL, "");
  if (N_str && EXPR_str)
  {
    long N = strtol(N_str, NULL, 10);
    bool success = false;
    uint32_t EXPR = expr(EXPR_str, &success);
    if (!success)
    {
      Log("cmd_scan: please check your expression");
      return -1;
    }
    Log("cmd_scan: x %ld 0x%08x\n", N, EXPR);
    long count = 0;
    for (long offset = 0; offset < N; offset++)
    {
      printf("0x");
      for (int i = 0; i < 4; i++)
      {
        printf("%02x", pmem[EXPR + offset * 4 + i]);
        count++;
      }
      printf(" ");
      if (count != 0 && count % 8 == 0)
      {
        printf("\n");
      }
    }
    return 0;
  }
  else
  {
    Log("cmd_scan: format error:  %s\n", args);
  }
  return -1;
}

static int cmd_detach(char *args) {
	difftest_detach();
	printf("Finish detaching.\n");
	return 0;
}

static int cmd_attach(char *args) {
	printf("Attaching...\n");
	difftest_attach();
	printf("Finish attaching.\n");
	return 0;
}

static int cmd_save(char *args) {
	char *arg = strtok(NULL, "");
	if (arg == NULL) {
		printf("Usage: save [path]\n");
		return 0;
	}
	isa_take_snapshot(arg);
	return 0;
}

static int cmd_load(char *args) {
	char *arg = strtok(NULL, "");
	if (arg == NULL) {
		printf("Usage: load [path]\n");
		return 0;
	}
	isa_recover_snapshot(arg);
	return 0;
}

// short name can be null, so check before used it
static struct
{
  char *name;
  char *short_name;
  char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "h", "Display informations about all supported commands", cmd_help},
    {"continue", "c", "Continue the execution of the program", cmd_c},
    {"quit", "q", "Exit NEMU", cmd_q},
    {"si", NULL, "Single step the program", cmd_si},
    {"info", "i", "info about the monitor", cmd_info},
    {"watchpoint", "w", "new watchpoint", cmd_new_wp},
    {"delete", "d", "remove watchpoint", cmd_rm_wp},
    {"print", "p", "Evaluate given expression", cmd_expr},
    {"scan", "x", "scan memory", cmd_scan},
    {"detach",NULL, "detach", cmd_detach},
    {"attach", NULL, "attach", cmd_attach},
    {"save", NULL, "save", cmd_save},
    {"load", NULL, "load", cmd_load},
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
      if (strcmp(arg, cmd_table[i].name) == 0 || (cmd_table[i].short_name != NULL && strcmp(arg, cmd_table[i].short_name) == 0))
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
      if (strcmp(cmd, cmd_table[i].name) == 0 || (cmd_table[i].short_name != NULL && strcmp(cmd_table[i].short_name, cmd) == 0))
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
