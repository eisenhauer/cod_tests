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
// /* 920730-1.c */
// #include <limits.h>
// f1()
// {
// 	int b=INT_MIN;
// 	return b>=INT_MIN;
// }
// 
// f2()
// {
// 	int b=INT_MIN+1;
// 	return b>= (unsigned)(INT_MAX+2);
// }
// 
// f3()
// {
// 	int b=INT_MAX;
// 	return b>=INT_MAX;
// }
// 
// f4()
// {
// 	int b=-1;
// 	return b>=UINT_MAX;
// }
// 
// main ()
// {
// 	if((f1()&f2()&f3()&f4())!=1)
// 		abort();
// 		exit(0);
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
	test_output = fopen("920730-1.c.output", "w");
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
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	#include <limits.h> f1();\n\
	void f2();\n\
	void f3();\n\
	void f4();\n\
	void main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
""};

    char *func_decls[] = {
	"#include <limits.h> f1();",
	"void f2();",
	"void f3();",
	"void f4();",
	"void main ();",
	""};

    char *func_bodies[] = {

/* body for f1 */
"{\n\
	int b=INT_MIN;\n\
	return b>=INT_MIN;\n\
}",

/* body for f2 */
"{\n\
	int b=INT_MIN+1;\n\
	return b>= (unsigned)(INT_MAX+2);\n\
}",

/* body for f3 */
"{\n\
	int b=INT_MAX;\n\
	return b>=INT_MAX;\n\
}",

/* body for f4 */
"{\n\
	int b=-1;\n\
	return b>=UINT_MAX;\n\
}",

/* body for main */
"{\n\
	if((f1()&f2()&f3()&f4())!=1)\n\
		abort();\n\
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
                printf("Test ./generated/920730-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 920730-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/920730-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/920730-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/920730-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/920730-1.c Succeeded\n");
    return 0;
}
