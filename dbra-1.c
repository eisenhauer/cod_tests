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
// int f1 (a)
//      long a;
// {
//   int i;
//   for (i = 0; i < 10; i++)
//     {
//       if (--a == -1)
// 	return i;
//     }
//   return -1;
// }
// 
// int f2 (a)
//      long a;
// {
//   int i;
//   for (i = 0; i < 10; i++)
//     {
//       if (--a != -1)
// 	return i;
//     }
//   return -1;
// }
// 
// int f3 (a)
//      long a;
// {
//   int i;
//   for (i = 0; i < 10; i++)
//     {
//       if (--a == 0)
// 	return i;
//     }
//   return -1;
// }
// 
// int f4 (a)
//      long a;
// {
//   int i;
//   for (i = 0; i < 10; i++)
//     {
//       if (--a != 0)
// 	return i;
//     }
//   return -1;
// }
// 
// int f5 (a)
//      long a;
// {
//   int i;
//   for (i = 0; i < 10; i++)
//     {
//       if (++a == 0)
// 	return i;
//     }
//   return -1;
// }
// 
// int f6 (a)
//      long a;
// {
//   int i;
//   for (i = 0; i < 10; i++)
//     {
//       if (++a != 0)
// 	return i;
//     }
//   return -1;
// }
// 
// 
// main()
// {
//   if (f1 (5L) != 5)
//     abort ();
//   if (f2 (1L) != 0)
//     abort ();
//   if (f2 (0L) != 1)
//     abort ();
//   if (f3 (5L) != 4)
//     abort ();
//   if (f4 (1L) != 1)
//     abort ();
//   if (f4 (0L) != 0)
//     abort ();
//   if (f5 (-5L) != 4)
//     abort ();
//   if (f6 (-1L) != 1)
//     abort ();
//   if (f6 (0L) != 0)
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
	test_output = fopen("dbra-1.c.output", "w");
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
	{"f1", (void*)(long)-1},
	{"f2", (void*)(long)-1},
	{"f3", (void*)(long)-1},
	{"f4", (void*)(long)-1},
	{"f5", (void*)(long)-1},
	{"f6", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	int f1 (long a);\n\
	int f2 (long a);\n\
	int f3 (long a);\n\
	int f4 (long a);\n\
	int f5 (long a);\n\
	int f6 (long a);\n\
	void main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
""};

    char *func_decls[] = {
	"int f1 (long a);",
	"int f2 (long a);",
	"int f3 (long a);",
	"int f4 (long a);",
	"int f5 (long a);",
	"int f6 (long a);",
	"void main();",
	""};

    char *func_bodies[] = {

/* body for f1 */
"{\n\
  int i;\n\
  for (i = 0; i < 10; i++)\n\
    {\n\
      if (--a == -1)\n\
	return i;\n\
    }\n\
  return -1;\n\
}",

/* body for f2 */
"{\n\
  int i;\n\
  for (i = 0; i < 10; i++)\n\
    {\n\
      if (--a != -1)\n\
	return i;\n\
    }\n\
  return -1;\n\
}",

/* body for f3 */
"{\n\
  int i;\n\
  for (i = 0; i < 10; i++)\n\
    {\n\
      if (--a == 0)\n\
	return i;\n\
    }\n\
  return -1;\n\
}",

/* body for f4 */
"{\n\
  int i;\n\
  for (i = 0; i < 10; i++)\n\
    {\n\
      if (--a != 0)\n\
	return i;\n\
    }\n\
  return -1;\n\
}",

/* body for f5 */
"{\n\
  int i;\n\
  for (i = 0; i < 10; i++)\n\
    {\n\
      if (++a == 0)\n\
	return i;\n\
    }\n\
  return -1;\n\
}",

/* body for f6 */
"{\n\
  int i;\n\
  for (i = 0; i < 10; i++)\n\
    {\n\
      if (++a != 0)\n\
	return i;\n\
    }\n\
  return -1;\n\
}",

/* body for main */
"{\n\
  if (f1 (5L) != 5)\n\
    abort ();\n\
  if (f2 (1L) != 0)\n\
    abort ();\n\
  if (f2 (0L) != 1)\n\
    abort ();\n\
  if (f3 (5L) != 4)\n\
    abort ();\n\
  if (f4 (1L) != 1)\n\
    abort ();\n\
  if (f4 (0L) != 0)\n\
    abort ();\n\
  if (f5 (-5L) != 4)\n\
    abort ();\n\
  if (f6 (-1L) != 1)\n\
    abort ();\n\
  if (f6 (0L) != 0)\n\
    abort ();\n\
  exit (0);\n\
}",
""};

    int i;
    cod_code gen_code[7];
    cod_parse_context context;
    for (i=0; i < 7; i++) {
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
        if (i == 6) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/dbra-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp dbra-1.c.output ./pre_patch/dbra-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/dbra-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/dbra-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/dbra-1.c Succeeded\n");
    return 0;
}
