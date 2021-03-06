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
// /* Originally added to test SH constant pool layout.  t1() failed for
//    non-PIC and t2() failed for PIC.  */
// 
// int t1 (float *f, int i,
// 	void (*f1) (double),
// 	void (*f2) (float, float))
// {
//   f1 (3.0);
//   f[i] = f[i + 1];
//   f2 (2.5f, 3.5f);
// }
// 
// int t2 (float *f, int i,
// 	void (*f1) (double),
// 	void (*f2) (float, float),
// 	void (*f3) (float))
// {
//   f3 (6.0f);
//   f1 (3.0);
//   f[i] = f[i + 1];
//   f2 (2.5f, 3.5f);
// }
// 
// void f1 (double d)
// {
//   if (d != 3.0)
//     abort ();
// }
// 
// void f2 (float f1, float f2)
// {
//   if (f1 != 2.5f || f2 != 3.5f)
//     abort ();
// }
// 
// void f3 (float f)
// {
//   if (f != 6.0f)
//     abort ();
// }
// 
// int main ()
// {
//   float f[3] = { 2.0f, 3.0f, 4.0f };
//   t1 (f, 0, f1, f2);
//   t2 (f, 1, f1, f2, f3);
//   if (f[0] != 3.0f && f[1] != 4.0f)
//     abort ();
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
	test_output = fopen("20021118-2.c.output", "w");
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
	{"t1", (void*)(long)-1},
	{"t2", (void*)(long)-1},
	{"f1", (void*)(long)-1},
	{"f2", (void*)(long)-1},
	{"f3", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	int t1 (float *f, int i, 	void (*f1) (double), 	void (*f2) (float, float));\n\
	int t2 (float *f, int i, 	void (*f1) (double), 	void (*f2) (float, float), 	void (*f3) (float));\n\
	void f1 (double d);\n\
	void f2 (float f1, float f2);\n\
	void f3 (float f);\n\
	int main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
""};

    char *func_decls[] = {
	"int t1 (float *f, int i, 	void (*f1) (double), 	void (*f2) (float, float));",
	"int t2 (float *f, int i, 	void (*f1) (double), 	void (*f2) (float, float), 	void (*f3) (float));",
	"void f1 (double d);",
	"void f2 (float f1, float f2);",
	"void f3 (float f);",
	"int main ();",
	""};

    char *func_bodies[] = {

/* body for t1 */
"{\n\
  f1 (3.0);\n\
  f[i] = f[i + 1];\n\
  f2 (2.5f, 3.5f);\n\
}",

/* body for t2 */
"{\n\
  f3 (6.0f);\n\
  f1 (3.0);\n\
  f[i] = f[i + 1];\n\
  f2 (2.5f, 3.5f);\n\
}",

/* body for f1 */
"{\n\
  if (d != 3.0)\n\
    abort ();\n\
}",

/* body for f2 */
"{\n\
  if (f1 != 2.5f || f2 != 3.5f)\n\
    abort ();\n\
}",

/* body for f3 */
"{\n\
  if (f != 6.0f)\n\
    abort ();\n\
}",

/* body for main */
"{\n\
  float f[3] = { 2.0f, 3.0f, 4.0f };\n\
  t1 (f, 0, f1, f2);\n\
  t2 (f, 1, f1, f2, f3);\n\
  if (f[0] != 3.0f && f[1] != 4.0f)\n\
    abort ();\n\
  exit (0);\n\
}",
""};

    int i;
    cod_code gen_code[6];
    cod_parse_context context;
    for (i=0; i < 6; i++) {
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
        if (i == 5) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/20021118-2.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20021118-2.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20021118-2.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20021118-2.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20021118-2.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20021118-2.c Succeeded\n");
    return 0;
}
