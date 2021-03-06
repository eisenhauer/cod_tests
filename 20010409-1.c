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
// typedef __SIZE_TYPE__ size_t;
// extern size_t strlen (const char *s);
// 
// typedef struct A {
//   int a, b;
// } A;
// 
// typedef struct B {
//   struct A **a;
//   int b;
// } B;
// 
// A *a;
// int b = 1, c;
// B d[1];
// 
// void foo (A *x, const char *y, int z)
// {
//   c = y[4] + z * 25;
// }
// 
// A *bar (const char *v, int w, int x, const char *y, int z)
// {
//   if (w)
//     abort ();
//   exit (0);
// }
// 
// void test (const char *x, int *y)
// {
//   foo (d->a[d->b], "test", 200);
//   d->a[d->b] = bar (x, b ? 0 : 65536, strlen (x), "test", 201);
//   d->a[d->b]->a++;
//   if (y)
//     d->a[d->b]->b = *y;
// }
// 
// int main ()
// {
//   d->b = 0;
//   d->a = &a;
//   test ("", 0);
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
	test_output = fopen("20010409-1.c.output", "w");
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
	{"foo", (void*)(long)-1},
	{"bar", (void*)(long)-1},
	{"test", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	void foo (A *x, const char *y, int z);\n\
	A *bar (const char *v, int w, int x, const char *y, int z);\n\
	void test (const char *x, int *y);\n\
	int main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
	"typedef __SIZE_TYPE__ size_t;\n\
extern size_t strlen (const char *s);",
	"typedef struct A {\n\
  int a, b;\n\
} A;",
	"typedef struct B {\n\
  struct A **a;\n\
  int b;\n\
} B;\n\
\n\
A *a;\n\
int b = 1, c;\n\
B d[1];",
""};

    char *func_decls[] = {
	"void foo (A *x, const char *y, int z);",
	"A *bar (const char *v, int w, int x, const char *y, int z);",
	"void test (const char *x, int *y);",
	"int main ();",
	""};

    char *func_bodies[] = {

/* body for foo */
"{\n\
  c = y[4] + z * 25;\n\
}",

/* body for bar */
"{\n\
  if (w)\n\
    abort ();\n\
  exit (0);\n\
}",

/* body for test */
"{\n\
  foo (d->a[d->b], \"test\", 200);\n\
  d->a[d->b] = bar (x, b ? 0 : 65536, strlen (x), \"test\", 201);\n\
  d->a[d->b]->a++;\n\
  if (y)\n\
    d->a[d->b]->b = *y;\n\
}",

/* body for main */
"{\n\
  d->b = 0;\n\
  d->a = &a;\n\
  test (\"\", 0);\n\
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
                printf("Test ./generated/20010409-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20010409-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20010409-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20010409-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20010409-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20010409-1.c Succeeded\n");
    return 0;
}
