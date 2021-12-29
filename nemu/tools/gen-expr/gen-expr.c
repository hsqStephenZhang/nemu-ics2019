#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#define BUFFER_SIZE 65536
#define len 10000

// this should be enough
static char buf[65536];

static int idx;

uint32_t choose(uint32_t x)
{
  return rand() % x;
}

void gen(uint32_t ch)
{
  buf[idx++] = ch;
}

void gen_num()
{
  char number_str[30];
  sprintf(number_str, "%d", rand());
  int length = strlen(number_str);
  for (int i = 0; i < length; i++)
  {
    buf[idx++] = number_str[i];
  }

  if (length != 0)
  {
    buf[idx++] = 'U';
  }
}

void gen_rand_op()
{
  switch (choose(4))
  {
  case 0:
    buf[idx++] = '+';
    break;
  case 1:
    buf[idx++] = '-';
    break;
  case 2:
    buf[idx++] = '*';
    break;
  default:
    buf[idx++] = '/';
    break;
  }
}

void gen_space(void)
{
  int temp = choose(3);
  for (int i = 0; i < temp; i++)
  {
    buf[idx++] = ' ';
  }
}

static inline void gen_rand_expr()
{
  if (idx > len)
  {
    gen_num();
    return;
  }
  gen_space();
  switch (choose(3))
  {
  case 0:
    gen_num();
    break;
  case 1:
    gen('(');
    gen_rand_expr();
    gen(')');
    break;
  default:
    gen_rand_expr();
    gen_rand_op();
    gen_rand_expr();
    break;
  }
}

static char code_buf[BUFFER_SIZE];
static char code_format[] =
    "#include <stdio.h>\n"
    "int main() { "
    "  unsigned result = %s; "
    "  printf(\"%%u\", result); "
    "  return 0; "
    "}";

int main(int argc, char *argv[])
{
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1)
  {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i++)
  {
    buf[0] = '\0';
    code_buf[0] = '\0';
    idx = 0;
    gen_rand_expr();
    buf[idx] = '\0';

    snprintf(code_buf, BUFFER_SIZE, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0)
      continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    __attribute__((unused)) int _res = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
