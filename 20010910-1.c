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
// /* Test case contributed by Ingo Rohloff <rohloff@in.tum.de>.
//    Code distilled from Linux kernel.  */
// 
// /* Compile this program with a gcc-2.95.2 using
//    "gcc -O2" and run it. The result will be that
//    rx_ring[1].next == 0   (it should be == 14)
//    and
//    ep.skbuff[4] == 5      (it should be 0)
// */
// 
// extern void abort(void);
// 
// struct epic_rx_desc 
// {
//   unsigned int next;
// };
// 
// struct epic_private 
// {
//   struct epic_rx_desc *rx_ring;
//   unsigned int rx_skbuff[5];
// };
// 
// static void epic_init_ring(struct epic_private *ep)
// {
//   int i;
// 
//   for (i = 0; i < 5; i++) 
//   {
//     ep->rx_ring[i].next = 10 + (i+1)*2;
//     ep->rx_skbuff[i] = 0;
//   }
//   ep->rx_ring[i-1].next = 10;
// }
// 
// static int check_rx_ring[5] = { 12,14,16,18,10 };
// 
// int main(void)
// {
//   struct epic_private ep;
//   struct epic_rx_desc rx_ring[5];
//   int i;
// 
//   for (i=0;i<5;i++)
//   {
//     rx_ring[i].next=0;
//     ep.rx_skbuff[i]=5;
//   }
//   
//   ep.rx_ring=rx_ring;
//   epic_init_ring(&ep);
//   
//   for (i=0;i<5;i++)
//   {
//     if ( rx_ring[i].next != check_rx_ring[i] ) abort();
//     if ( ep.rx_skbuff[i] != 0 ) abort();
//   }
//   return 0;
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
	test_output = fopen("20010910-1.c.output", "w");
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
	{"epic_init_ring", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	static void epic_init_ring(struct epic_private *ep);\n\
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
	"struct epic_rx_desc{\n\
  unsigned int next;\n\
};",
	"struct epic_private{\n\
  struct epic_rx_desc *rx_ring;\n\
  unsigned int rx_skbuff[5];\n\
};",
	"static int check_rx_ring[5] ={ 12,14,16,18,10 };",
""};

    char *func_decls[] = {
	"static void epic_init_ring(struct epic_private *ep);",
	"int main();",
	""};

    char *func_bodies[] = {

/* body for epic_init_ring */
"{\n\
  int i;\n\
\n\
  for (i = 0; i < 5; i++) \n\
  {\n\
    ep->rx_ring[i].next = 10 + (i+1)*2;\n\
    ep->rx_skbuff[i] = 0;\n\
  }\n\
  ep->rx_ring[i-1].next = 10;\n\
}",

/* body for main */
"{\n\
  struct epic_private ep;\n\
  struct epic_rx_desc rx_ring[5];\n\
  int i;\n\
\n\
  for (i=0;i<5;i++)\n\
  {\n\
    rx_ring[i].next=0;\n\
    ep.rx_skbuff[i]=5;\n\
  }\n\
  \n\
  ep.rx_ring=rx_ring;\n\
  epic_init_ring(&ep);\n\
  \n\
  for (i=0;i<5;i++)\n\
  {\n\
    if ( rx_ring[i].next != check_rx_ring[i] ) abort();\n\
    if ( ep.rx_skbuff[i] != 0 ) abort();\n\
  }\n\
  return 0;\n\
}",
""};

    int i;
    cod_code gen_code[2];
    cod_parse_context context;
    for (i=0; i < 2; i++) {
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
        if (i == 1) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/20010910-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20010910-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20010910-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20010910-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20010910-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20010910-1.c Succeeded\n");
    return 0;
}
