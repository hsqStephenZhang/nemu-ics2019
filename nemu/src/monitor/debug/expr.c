#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

uint32_t isa_reg_str2val(const char *, bool *);

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
    {" +", TK_NOTYPE}, // spaces
    {"\\*", '*'},      // multi
    {"/", '/'},        // divide
    {"\\+", '+'},      // plus
    {"\\-", '-'},      // sub
    {"\\(", '('},
    {"\\)", ')'},
    {"0x[0-9a-f]+U", TK_HEX_NUM},      // hex nums
    {"0x[0-9a-f]+", TK_HEX_NUM},       // hex nums, ignore the 'U'
    {"[0-9]+U", TK_DECI_NUM},          // decimal nums
    {"[0-9]+", TK_DECI_NUM},           // decimal nums, ignore the 'U'
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

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
        //     i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

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

// check if a expression matches a pair of parentheses
bool check_parentheses(int p, int q)
{
  int count = 0;
  for (int i = p + 1; i <= q - 1; i++)
  {
    if (tokens[i].type == '(')
    {
      count += 1;
    }
    else if (tokens[i].type == ')')
    {
      count -= 1;
    }

    if (count < 0)
    {
      return false;
    }
  }

  return count == 0;
}

int locate_op(int p, int q, bool *success)
{
}

/*
 * p: left index
 * q: right index
 */
uint32_t eval(int p, int q, bool *success)
{
  if (p > q)
  {
    /* Bad expression */
    return -1;
  }
  else if (p == q)
  {
    if (tokens[p].type == TK_HEX_NUM)
    {
       return strtol(tokens[p].str, NULL, 16);
    }
    else if (tokens[p].type == TK_DECI_NUM)
    {
       return strtol(tokens[p].str, NULL, 10);
    }
    else if (tokens[p].type == TK_REG)
    {
       uint32_t reg_val = isa_reg_str2val(tokens[p].str, success);
       if (*success){
         return reg_val;
       }
       return -1;
    }else {
      TODO();
    }
  }
  else if ((tokens[p].type == '(' && tokens[q].type == ')') && check_parentheses(p, q) == true)
  {
    return eval(p + 1, q - 1, success);
  }
  else
  {
    // if (*success==false){
    //   Log("check parentheses failed");
    //   return -1;
    // }
    // op = the position of 主运算符 in the token expression;
    int op = 1;
    uint32_t val1 = eval(p, op - 1, success);
    uint32_t val2 = eval(op + 1, q, success);

    switch (tokens[op].type)
    {
    case '+':
      return val1 + val2;
    case '-':
      return val1 - val2;
    case '*':
      return val1 * val2;
    case '/':
    {
      // TODO: divide by zero trap
      if (val2 == 0)
      {
        Log("divide by zero error");
        return -1;
      }
      else
      {
        return val1 / val2;
      }
    }
    default: panic("unknown operation");
    }
  }
}

uint32_t expr(char *e, bool *success)
{
  if (!make_token(e))
  {
    *success = false;
    return 0;
  }

  Log("eval expression...");

  for (int i = 0; i < nr_token; i++)
  {
    printf("%d", tokens[i].type);
    if (tokens[i].type == TK_DECI_NUM || tokens[i].type == TK_HEX_NUM || tokens[i].type == TK_REG)
    {
      printf("%s\n", tokens[i].str);
    }
    else
    {
      printf("\n");
    }
  }

  uint32_t res = eval(0, nr_token - 1, success);

  return res;
}
