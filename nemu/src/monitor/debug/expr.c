#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum
{
  TK_NOTYPE = 256,
  TK_EQ,       // ==
  TK_DECI_NUM, // 十进制数字
  TK_HEX_NUM,  // 十六进制数字
  TK_REG,      // 寄存器
  TK_DEREF,    // 解引用
  TK_NTEQ,     // 不等于
  TK_AND,      // &&
};

static struct rule
{
  char *regex;
  int token_type;
} rules[] = {

    /* TODO: Add more rules.
     * Pay attention to the precedence level of different rules.
     */

    {" +", TK_NOTYPE}, // spaces
    {"\\*", '*'},      // multi
    {"/", '/'},        // divide
    {"\\+", '+'},      // plus
    {"\\-", '-'},      // sub
    {"==", TK_EQ},     // equal
    {"!=", TK_NTEQ},   // not equal
    {"&&", TK_AND},    // and &&
    {"\\(", '('},
    {"\\)", ')'},
    {"0x[0-9a-f]+U", TK_HEX_NUM},      // hex nums
    {"[0-9]+U", TK_DECI_NUM},          // decimal nums
    {"[\\$][a-zA-Z0-9]{1,3}", TK_REG}, // the length of the reg's name should be 2 or 3
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i++)
  {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0)
    {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token
{
  int type;
  char str[32];
} Token;

static Token tokens[1024] __attribute__((used)) = {};
static int nr_token __attribute__((used)) = 0;

static bool make_token(char *e)
{
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  Log("makeing token...");

  while (e[position] != '\0')
  {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i++)
    {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)
      {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type)
        {
        case TK_HEX_NUM:
        case TK_DECI_NUM:
          strncpy(tokens[nr_token].str, substr_start, substr_len);
          tokens[nr_token].str[substr_len] = '\0';
          break;
        case TK_REG:
          // skip the very first '$' of register
          strncpy(tokens[nr_token].str, substr_start + 1, substr_len - 1);
          tokens[nr_token].str[substr_len - 1] = '\0';
        default:
          break;
          // ignore other tokens, because we only need its token_type
        }

        if (rules[i].token_type != TK_NOTYPE)
        {
          tokens[nr_token++].type = rules[i].token_type;
        }

        break;
      }
    }

    if (i == NR_REGEX)
    {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

uint32_t expr(char *e, bool *success)
{
  if (!make_token(e))
  {
    *success = false;
    return 0;
  }

  // Log("eval expression...");

  /* TODO: Insert codes to evaluate the expression. */
  // for (int i = 0; i < nr_token; i++)
  // {
  //   printf("%d", tokens[i].type);
  //   if (tokens[i].type == TK_DECI_NUM || tokens[i].type == TK_HEX_NUM || tokens[i].type == TK_REG)
  //   {
  //     printf("%s\n", tokens[i].str);
  //   }
  //   else
  //   {
  //     printf("\n");
  //   }
  // }

  return 0;
}
