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
// #include <stdio.h>
// 
// void fred()
// {
//    printf("In fred()\n");
//    goto done;
//    printf("In middle\n");
// done:
//    printf("At end\n");
// }
// 
// void joe()
// {
//    int b = 5678;
// 
//    printf("In joe()\n");
// 
//    {
//       int c = 1234;
//       printf("c = %d\n", c);
//       goto outer;
//       printf("uh-oh\n");
//    }
// 
// outer:    
// 
//    printf("done\n");
// }
// 
// void henry()
// {
//    int a;
// 
//    printf("In henry()\n");
//    goto inner;
// 
//    {
//       int b;
// inner:    
//       b = 1234;
//       printf("b = %d\n", b);
//    }
// 
//    printf("done\n");
// }
// 
// int main()
// {
//    fred();
//    joe();
//    henry();
// 
//    return 0;
// }
// 
// /* vim: set expandtab ts=4 sw=3 sts=3 tw=80 :*/

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
	test_output = fopen("54_goto.c.output", "w");
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
	{"fred", (void*)(long)-1},
	{"joe", (void*)(long)-1},
	{"henry", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	void fred();\n\
	void joe();\n\
	void henry();\n\
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
	"#include <stdio.h>",
""};

    char *func_decls[] = {
	"void fred();",
	"void joe();",
	"void henry();",
	"int main();",
	""};

    char *func_bodies[] = {

/* body for fred */
"{\n\
   test_printf(\"In fred()\\n\");\n\
   goto done;\n\
   test_printf(\"In middle\\n\");\n\
done:\n\
   test_printf(\"At end\\n\");\n\
}",

/* body for joe */
"{\n\
   int b = 5678;\n\
\n\
   test_printf(\"In joe()\\n\");\n\
\n\
   {\n\
      int c = 1234;\n\
      test_printf(\"c = %d\\n\", c);\n\
      goto outer;\n\
      test_printf(\"uh-oh\\n\");\n\
   }\n\
\n\
outer:    \n\
\n\
   test_printf(\"done\\n\");\n\
}",

/* body for henry */
"{\n\
   int a;\n\
\n\
   test_printf(\"In henry()\\n\");\n\
   goto inner;\n\
\n\
   {\n\
      int b;\n\
inner:    \n\
      b = 1234;\n\
      test_printf(\"b = %d\\n\", b);\n\
   }\n\
\n\
   test_printf(\"done\\n\");\n\
}",

/* body for main */
"{\n\
   fred();\n\
   joe();\n\
   henry();\n\
\n\
   return 0;\n\
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
                printf("Test ./generated/54_goto.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 54_goto.c.output /Users/eisen/prog/tinycc/tests/tests2/54_goto.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/54_goto.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/54_goto.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/54_goto.c Succeeded\n");
    return 0;
}
