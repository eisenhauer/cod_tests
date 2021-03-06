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
// long long proc1(){return 1LL;}
// long long proc2(){return 0x12345678LL;}
// long long proc3(){return 0xaabbccdd12345678LL;}
// long long proc4(){return -1LL;}
// long long proc5(){return 0xaabbccddLL;}
// 
// print_longlong(x,buf)
//      long long x;
//      char *buf;
// {
//   unsigned long l;
//   l= (x >> 32) & 0xffffffff;
//   if (l != 0)
//     sprintf(buf,"%lx%08.lx",l,((unsigned long)x & 0xffffffff));
//   else
//     sprintf(buf,"%lx",((unsigned long)x & 0xffffffff));
// }
// 
// main(){char buf[100];
// print_longlong(proc1(),buf);if(strcmp("1",buf))abort();
// print_longlong(proc2(),buf);if(strcmp("12345678",buf))abort();
// print_longlong(proc3(),buf);if(strcmp("aabbccdd12345678",buf))abort();
// print_longlong(proc4(),buf);if(strcmp("ffffffffffffffff",buf))abort();
// print_longlong(proc5(),buf);if(strcmp("aabbccdd",buf))abort();
// exit(0);}

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
	test_output = fopen("920501-9.c.output", "w");
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
	{"proc1", (void*)(long)-1},
	{"proc2", (void*)(long)-1},
	{"proc3", (void*)(long)-1},
	{"proc4", (void*)(long)-1},
	{"proc5", (void*)(long)-1},
	{"print_longlong", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	long long proc1();\n\
	long long proc2();\n\
	long long proc3();\n\
	long long proc4();\n\
	long long proc5();\n\
	void print_longlong(long long x, char *buf);\n\
	void main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
	"#include <stdio.h>",
""};

    char *func_decls[] = {
	"long long proc1();",
	"long long proc2();",
	"long long proc3();",
	"long long proc4();",
	"long long proc5();",
	"void print_longlong(long long x, char *buf);",
	"void main();",
	""};

    char *func_bodies[] = {

/* body for proc1 */
"{return 1LL;}",

/* body for proc2 */
"{return 0x12345678LL;}",

/* body for proc3 */
"{return 0xaabbccdd12345678LL;}",

/* body for proc4 */
"{return -1LL;}",

/* body for proc5 */
"{return 0xaabbccddLL;}",

/* body for print_longlong */
"{\n\
  unsigned long l;\n\
  l= (x >> 32) & 0xffffffff;\n\
  if (l != 0)\n\
    sprintf(buf,\"%lx%08.lx\",l,((unsigned long)x & 0xffffffff));\n\
  else\n\
    sprintf(buf,\"%lx\",((unsigned long)x & 0xffffffff));\n\
}",

/* body for main */
"{char buf[100];\n\
print_longlong(proc1(),buf);if(strcmp(\"1\",buf))abort();\n\
print_longlong(proc2(),buf);if(strcmp(\"12345678\",buf))abort();\n\
print_longlong(proc3(),buf);if(strcmp(\"aabbccdd12345678\",buf))abort();\n\
print_longlong(proc4(),buf);if(strcmp(\"ffffffffffffffff\",buf))abort();\n\
print_longlong(proc5(),buf);if(strcmp(\"aabbccdd\",buf))abort();\n\
exit(0);}",
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
                printf("Test ./generated/920501-9.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 920501-9.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/920501-9.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/920501-9.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/920501-9.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/920501-9.c Succeeded\n");
    return 0;
}
