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
// void dummy () 
// {
// }
// 
// int f1 (unsigned int x, unsigned int y)
// {
//   if (x == 0)
//     dummy ();
//   x -= y;
//   /* 0xfffffff2 < 0x80000000? */
//   if (x < ~(~(unsigned int) 0 >> 1))
//     abort ();
//   return x;
// }
// 
// int f2 (unsigned long int x, unsigned long int y)
// {
//   if (x == 0)
//     dummy ();
//   x -= y;
//   /* 0xfffffff2 < 0x80000000? */
//   if (x < ~(~(unsigned long int) 0 >> 1))
//     abort ();
//   return x;
// }
// 
// 
// main ()
// {
//   /*      0x7ffffff3			0x80000001 */
//   f1 ((~(unsigned int) 0 >> 1) - 12, ~(~(unsigned int) 0 >> 1) + 1);
//   f2 ((~(unsigned long int) 0 >> 1) - 12, ~(~(unsigned long int) 0 >> 1) + 1);
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
	test_output = fopen("cmpsi-1.c.output", "w");
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
	{"dummy", (void*)(long)-1},
	{"f1", (void*)(long)-1},
	{"f2", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	void dummy ();\n\
	int f1 (unsigned int x, unsigned int y);\n\
	int f2 (unsigned long int x, unsigned long int y);\n\
	void main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
""};

    char *func_decls[] = {
	"void dummy ();",
	"int f1 (unsigned int x, unsigned int y);",
	"int f2 (unsigned long int x, unsigned long int y);",
	"void main ();",
	""};

    char *func_bodies[] = {

/* body for dummy */
"{\n\
}",

/* body for f1 */
"{\n\
  if (x == 0)\n\
    dummy ();\n\
  x -= y;\n\
  /* 0xfffffff2 < 0x80000000? */\n\
  if (x < ~(~(unsigned int) 0 >> 1))\n\
    abort ();\n\
  return x;\n\
}",

/* body for f2 */
"{\n\
  if (x == 0)\n\
    dummy ();\n\
  x -= y;\n\
  /* 0xfffffff2 < 0x80000000? */\n\
  if (x < ~(~(unsigned long int) 0 >> 1))\n\
    abort ();\n\
  return x;\n\
}",

/* body for main */
"{\n\
  /*      0x7ffffff3			0x80000001 */\n\
  f1 ((~(unsigned int) 0 >> 1) - 12, ~(~(unsigned int) 0 >> 1) + 1);\n\
  f2 ((~(unsigned long int) 0 >> 1) - 12, ~(~(unsigned long int) 0 >> 1) + 1);\n\
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
                printf("Test ./generated/cmpsi-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp cmpsi-1.c.output ./pre_patch/cmpsi-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/cmpsi-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/cmpsi-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/cmpsi-1.c Succeeded\n");
    return 0;
}
