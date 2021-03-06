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
// struct A
// {
//   unsigned long p, q, r, s;
// } x = { 13, 14, 15, 16 };
// 
// extern void abort (void);
// extern void exit (int);
// 
// static inline struct A *
// bar (void)
// {
//   struct A *r;
// 
//   switch (8)
//     {
//     case 2:
//       abort ();
//       break;
//     case 8:
//       r = &x;
//       break;
//     default:
//       abort ();
//       break;
//     }
//   return r;
// }
// 
// void
// foo (unsigned long *x, int y)
// {
//   if (y != 12)
//     abort ();
//   if (x[0] != 1 || x[1] != 11)
//     abort ();
//   if (x[2] != 2 || x[3] != 12)
//     abort ();
//   if (x[4] != 3 || x[5] != 13)
//     abort ();
//   if (x[6] != 4 || x[7] != 14)
//     abort ();
//   if (x[8] != 5 || x[9] != 15)
//     abort ();
//   if (x[10] != 6 || x[11] != 16)
//     abort ();
// }
// 
// int
// main (void)
// {
//   unsigned long a[40];
//   int b = 0;
// 
//   a[b++] = 1;
//   a[b++] = 11;
//   a[b++] = 2;
//   a[b++] = 12;
//   a[b++] = 3;
//   a[b++] = bar()->p;
//   a[b++] = 4;
//   a[b++] = bar()->q;
//   a[b++] = 5;
//   a[b++] = bar()->r;
//   a[b++] = 6;
//   a[b++] = bar()->s;
//   foo (a, b);
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
	test_output = fopen("20030313-1.c.output", "w");
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
	{"bar", (void*)(long)-1},
	{"foo", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	static inline struct A * bar ();\n\
	void foo (unsigned long *x, int y);\n\
	int main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
	"struct A\n\
{\n\
  unsigned long p, q, r, s;\n\
} x = { 13, 14, 15, 16 };\n\
\n\
\n\
",
""};

    char *func_decls[] = {
	"static inline struct A * bar ();",
	"void foo (unsigned long *x, int y);",
	"int main ();",
	""};

    char *func_bodies[] = {

/* body for bar */
"{\n\
  struct A *r;\n\
\n\
  switch (8)\n\
    {\n\
    case 2:\n\
      abort ();\n\
      break;\n\
    case 8:\n\
      r = &x;\n\
      break;\n\
    default:\n\
      abort ();\n\
      break;\n\
    }\n\
  return r;\n\
}",

/* body for foo */
"{\n\
  if (y != 12)\n\
    abort ();\n\
  if (x[0] != 1 || x[1] != 11)\n\
    abort ();\n\
  if (x[2] != 2 || x[3] != 12)\n\
    abort ();\n\
  if (x[4] != 3 || x[5] != 13)\n\
    abort ();\n\
  if (x[6] != 4 || x[7] != 14)\n\
    abort ();\n\
  if (x[8] != 5 || x[9] != 15)\n\
    abort ();\n\
  if (x[10] != 6 || x[11] != 16)\n\
    abort ();\n\
}",

/* body for main */
"{\n\
  unsigned long a[40];\n\
  int b = 0;\n\
\n\
  a[b++] = 1;\n\
  a[b++] = 11;\n\
  a[b++] = 2;\n\
  a[b++] = 12;\n\
  a[b++] = 3;\n\
  a[b++] = bar()->p;\n\
  a[b++] = 4;\n\
  a[b++] = bar()->q;\n\
  a[b++] = 5;\n\
  a[b++] = bar()->r;\n\
  a[b++] = 6;\n\
  a[b++] = bar()->s;\n\
  foo (a, b);\n\
  exit (0);\n\
}",
""};

    int i;
    cod_code gen_code[3];
    cod_parse_context context;
    for (i=0; i < 3; i++) {
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
        if (i == 2) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/20030313-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20030313-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20030313-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20030313-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20030313-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20030313-1.c Succeeded\n");
    return 0;
}
