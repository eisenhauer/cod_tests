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
// static inline fu (unsigned short data)
// {
//   return data;
// }
// ru(i)
// {
//    if(fu(i++)!=5)abort();
//    if(fu(++i)!=7)abort();
// }
// static inline fs (signed short data)
// {
//   return data;
// }
// rs(i)
// {
//    if(fs(i++)!=5)abort();
//    if(fs(++i)!=7)abort();
// }
// 
// 
// main()
// {
//   ru(5);
//   rs(5);
//   exit(0);
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
	test_output = fopen("920721-3.c.output", "w");
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
	{"fu", (void*)(long)-1},
	{"ru", (void*)(long)-1},
	{"fs", (void*)(long)-1},
	{"rs", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	static inline fu (unsigned short data);\n\
	ru(i);\n\
	static inline fs (signed short data);\n\
	rs(i);\n\
	void main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
""};

    char *func_decls[] = {
	"static inline fu (unsigned short data);",
	"ru(i);",
	"static inline fs (signed short data);",
	"rs(i);",
	"void main();",
	""};

    char *func_bodies[] = {

/* body for fu */
"{\n\
  return data;\n\
}",

/* body for ru */
"{\n\
   if(fu(i++)!=5)abort();\n\
   if(fu(++i)!=7)abort();\n\
}",

/* body for fs */
"{\n\
  return data;\n\
}",

/* body for rs */
"{\n\
   if(fs(i++)!=5)abort();\n\
   if(fs(++i)!=7)abort();\n\
}",

/* body for main */
"{\n\
  ru(5);\n\
  rs(5);\n\
  exit(0);\n\
}",
""};

    int i;
    cod_code gen_code[5];
    cod_parse_context context;
    for (i=0; i < 5; i++) {
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
        if (i == 4) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/920721-3.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 920721-3.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/920721-3.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/920721-3.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/920721-3.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/920721-3.c Succeeded\n");
    return 0;
}
