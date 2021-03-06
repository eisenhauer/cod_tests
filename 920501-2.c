#include "cod.h"
#undef NDEBUG
#include "assert.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>

/*
 *  Original test was:
 */
// unsigned long
// gcd_ll (unsigned long long x, unsigned long long y)
// {
//   for (;;)
//     {
//       if (y == 0)
// 	return (unsigned long) x;
//       x = x % y;
//       if (x == 0)
// 	return (unsigned long) y;
//       y = y % x;
//     }
//   return 0;
// }
// 
// unsigned long long
// powmod_ll (unsigned long long b, unsigned e, unsigned long long m)
// {
//   unsigned t;
//   unsigned long long pow;
//   int i;
// 
//   if (e == 0)
//     return 1;
// 
//   /* Find the most significant bit in E.  */
//   t = e;
//   for (i = 0; t != 0; i++)
//     t >>= 1;
// 
//   /* The most sign bit in E is handled outside of the loop, by beginning
//      with B in POW, and decrementing I.  */
//   pow = b;
//   i -= 2;
// 
//   for (; i >= 0; i--)
//     {
//       pow = pow * pow % m;
//       if ((1 << i) & e)
// 	pow = pow * b % m;
//     }
// 
//   return pow;
// }
// 
// unsigned long factab[10];
// 
// void
// facts (t, a_int, x0, p)
//      unsigned long long t;
//      int a_int;
//      int x0;
//      unsigned p;
// {
//   unsigned long *xp = factab;
//   unsigned long long x, y;
//   unsigned long q = 1;
//   unsigned long long a = a_int;
//   int i;
//   unsigned long d;
//   int j = 1;
//   unsigned long tmp;
//   int jj = 0;
// 
//   x = x0;
//   y = x0;
// 
//   for (i = 1; i < 10000; i++)
//     {
//       x = powmod_ll (x, p, t) + a;
//       y = powmod_ll (y, p, t) + a;
//       y = powmod_ll (y, p, t) + a;
// 
//       if (x > y)
// 	tmp = x - y;
//       else
// 	tmp = y - x;
//       q = (unsigned long long) q * tmp % t;
// 
//       if (i == j)
// 	{
// 	  jj += 1;
// 	  j += jj;
// 	  d = gcd_ll (q, t);
// 	  if (d != 1)
// 	    {
// 	      *xp++ = d;
// 	      t /= d;
// 	      if (t == 1)
// 		{
// 		  return;
// 		  *xp = 0;
// 		}
// 	    }
// 	}
//     }
// }
// 
// main ()
// {
//   unsigned long long t;
//   unsigned x0, a;
//   unsigned p;
// 
//   p = 27;
//   t = (1ULL << p) - 1;
// 
//   a = -1;
//   x0 = 3;
// 
//   facts (t, a, x0, p);
//   if (factab[0] != 7 || factab[1] != 73 || factab[2] != 262657)
//     abort();
//   exit (0);
// }

int exit_value = 0; /* success */
jmp_buf env;

void
my_abort()
{
    exit_value = 1; /* failure */
    longjmp(env, 1);
}

void
test_exit(int value)
{
    if (value != 0) {
	exit_value = value;
    }
    longjmp(env, 1);
}

FILE *test_output = NULL;
int verbose = 0;

int test_printf(const char *format, ...)
{
    int ret;
    va_list args;
    va_start(args, format);

    if (test_output == NULL) {
	test_output = fopen("920501-2.c.output", "w");
    }
    if (verbose) vprintf(format, args);
    ret = vfprintf(test_output, format, args);

    va_end(args);
    return ret;
}

int
main(int argc, char**argv)
{
    while (argc > 1) {
	if (strcmp(argv[1], "-v") == 0) {
	    verbose++;
        }
	argc--; argv++;
    }
    cod_extern_entry externs[] = 
    {
	{"gcd_ll", (void*)(long)-1},
	{"powmod_ll", (void*)(long)-1},
	{"facts", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	unsigned long gcd_ll (unsigned long long x, unsigned long long y);\n\
	unsigned long long powmod_ll (unsigned long long b, unsigned e, unsigned long long m);\n\
	void facts (unsigned long long t, int a_int, int x0, unsigned p);\n\
	void main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
	"unsigned long factab[10];",
""};

    char *func_decls[] = {
	"unsigned long gcd_ll (unsigned long long x, unsigned long long y);",
	"unsigned long long powmod_ll (unsigned long long b, unsigned e, unsigned long long m);",
	"void facts (unsigned long long t, int a_int, int x0, unsigned p);",
	"void main ();",
	""};

    char *func_bodies[] = {

/* body for gcd_ll */
"{\n\
  for (;;)\n\
    {\n\
      if (y == 0)\n\
	return (unsigned long) x;\n\
      x = x % y;\n\
      if (x == 0)\n\
	return (unsigned long) y;\n\
      y = y % x;\n\
    }\n\
  return 0;\n\
}",

/* body for powmod_ll */
"{\n\
  unsigned t;\n\
  unsigned long long pow;\n\
  int i;\n\
\n\
  if (e == 0)\n\
    return 1;\n\
\n\
  /* Find the most significant bit in E.  */\n\
  t = e;\n\
  for (i = 0; t != 0; i++)\n\
    t >>= 1;\n\
\n\
  /* The most sign bit in E is handled outside of the loop, by beginning\n\
     with B in POW, and decrementing I.  */\n\
  pow = b;\n\
  i -= 2;\n\
\n\
  for (; i >= 0; i--)\n\
    {\n\
      pow = pow * pow % m;\n\
      if ((1 << i) & e)\n\
	pow = pow * b % m;\n\
    }\n\
\n\
  return pow;\n\
}",

/* body for facts */
"{\n\
  unsigned long *xp = factab;\n\
  unsigned long long x, y;\n\
  unsigned long q = 1;\n\
  unsigned long long a = a_int;\n\
  int i;\n\
  unsigned long d;\n\
  int j = 1;\n\
  unsigned long tmp;\n\
  int jj = 0;\n\
\n\
  x = x0;\n\
  y = x0;\n\
\n\
  for (i = 1; i < 10000; i++)\n\
    {\n\
      x = powmod_ll (x, p, t) + a;\n\
      y = powmod_ll (y, p, t) + a;\n\
      y = powmod_ll (y, p, t) + a;\n\
\n\
      if (x > y)\n\
	tmp = x - y;\n\
      else\n\
	tmp = y - x;\n\
      q = (unsigned long long) q * tmp % t;\n\
\n\
      if (i == j)\n\
	{\n\
	  jj += 1;\n\
	  j += jj;\n\
	  d = gcd_ll (q, t);\n\
	  if (d != 1)\n\
	    {\n\
	      *xp++ = d;\n\
	      t /= d;\n\
	      if (t == 1)\n\
		{\n\
		  return;\n\
		  *xp = 0;\n\
		}\n\
	    }\n\
	}\n\
    }\n\
}",

/* body for main */
"{\n\
  unsigned long long t;\n\
  unsigned x0, a;\n\
  unsigned p;\n\
\n\
  p = 27;\n\
  t = (1ULL << p) - 1;\n\
\n\
  a = -1;\n\
  x0 = 3;\n\
\n\
  facts (t, a, x0, p);\n\
  if (factab[0] != 7 || factab[1] != 73 || factab[2] != 262657)\n\
    abort();\n\
  exit (0);\n\
}",
""};

    int i;
    cod_code gen_code[4];
    cod_parse_context context;
    for (i=0; i < 4; i++) {
        int j;
        if (verbose) {
             printf("Working on subroutine %s\n", externs[i].extern_name);
        }
        if (i==0) {
            context = new_cod_parse_context();
            cod_assoc_externs(context, externs);
            for (j=0; j < sizeof(global_decls)/sizeof(global_decls[0])-1; j++) {
                cod_parse_for_globals(global_decls[j], context);
            }
            cod_parse_for_context(extern_string, context);
        } else {
	    cod_extern_entry single_extern[2];
	    single_extern[0] = externs[i-1];
	    single_extern[1].extern_name = NULL;
	    single_extern[1].extern_value = NULL;
	    cod_assoc_externs(context, single_extern);
	    cod_parse_for_context(func_decls[i-1], context);
	}
        cod_subroutine_declaration(func_decls[i], context);
        gen_code[i] = cod_code_gen(func_bodies[i], context);
        externs[i].extern_value = (void*) gen_code[i]->func;
        if (i == 3) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/920501-2.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 920501-2.c.output ./pre_patch/920501-2.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/920501-2.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/920501-2.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/920501-2.c Succeeded\n");
    return 0;
}
