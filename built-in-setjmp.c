#include "cod.h"
#undef NDEBUG
#include "assert.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <setjmp.h>

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
	test_output = fopen("built-in-setjmp.c.output", "w");
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
	{"__attribute__", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	void __attribute__((noinline)) sub2 ();\n\
	int main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for __attribute__ */
"{\n\
  __builtin_longjmp (buf, 1);\n\
}",

/* body for main */
"{\n\
  char *p = (char *) __builtin_alloca (20);\n\
\n\
  strcpy (p, \"test\");\n\
\n\
  if (__builtin_setjmp (buf))\n\
    {\n\
      if (strcmp (p, \"test\") != 0)\n\
	abort ();\n\
\n\
      exit (0);\n\
    }\n\
\n\
  {\n\
    int *q = (int *) __builtin_alloca (p[2] * sizeof (int));\n\
    int i;\n\
    \n\
    for (i = 0; i < p[2]; i++)\n\
      q[i] = 0;\n\
\n\
    while (1)\n\
      sub2 ();\n\
  }\n\
}",
""};

    char *func_decls[] = {
	"void __attribute__((noinline)) sub2 ();",
	"int main ();",
	""};

    char *global_decls[] = {
	"extern int strcmp(const char *, const char *);\n\
extern char *strcpy(char *, const char *);\n\
extern void abort(void);\n\
extern void exit(int);\n\
\n\
void *buf[20];",
""};

    int i;
    cod_code gen_code[2];
    for (i=0; i < 2; i++) {
        int j;
        if (verbose) {
             printf("Working on subroutine %s\n", externs[i].extern_name);
        }
        cod_parse_context context = new_cod_parse_context();
        cod_assoc_externs(context, externs);
        for (j=0; j < 1; j++) {
            cod_parse_for_globals(global_decls[j], context);
        }
        cod_parse_for_context(extern_string, context);
        cod_subroutine_declaration(func_decls[i], context);
        gen_code[i] = cod_code_gen(func_bodies[i], context);
        externs[i].extern_value = (void*) gen_code[i]->func;
        if (i == 1) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/built-in-setjmp.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp built-in-setjmp.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/built-in-setjmp.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/built-in-setjmp.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/built-in-setjmp.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/built-in-setjmp.c Succeeded\n");
    return 0;
}