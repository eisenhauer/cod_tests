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
	test_output = fopen("991201-1.c.output", "w");
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
	{"reset_palette", (void*)(long)-1},
	{"bar", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	void reset_palette(int currcons);\n\
	void bar(int k);\n\
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for reset_palette */
"{\n\
	int j, k;\n\
	for (j=k=0; j<16; j++) {\n\
		(vc_cons[currcons].d->vc_palette) [k++] = default_red[j];\n\
		(vc_cons[currcons].d->vc_palette) [k++] = default_grn[j];\n\
		(vc_cons[currcons].d->vc_palette) [k++] = default_blu[j];\n\
	}\n\
	bar(k);\n\
}",

/* body for bar */
"{\n\
	if (k != 16*3)\n\
		abort();\n\
}",

/* body for main */
"{\n\
	reset_palette(0);\n\
	exit(0);\n\
}",
""};

    char *func_decls[] = {
	"void reset_palette(int currcons);",
	"void bar(int k);",
	"int main();",
	""};

    char *global_decls[] = {
	"struct vc_data {\n\
	unsigned long	space;\n\
	unsigned char   vc_palette[16*3];        \n\
};",
	"struct vc {\n\
	struct vc_data *d;\n\
};\n\
\n\
struct vc_data a_con;",
	"struct vc vc_cons[63] =",
	";\n\
int default_red[16];\n\
int default_grn[16];\n\
int default_blu[16];\n\
\n\
extern void bar(int);",
""};

    int i;
    cod_code gen_code[3];
    for (i=0; i < 3; i++) {
        int j;
        if (verbose) {
             printf("Working on subroutine %s\n", externs[i].extern_name);
        }
        cod_parse_context context = new_cod_parse_context();
        cod_assoc_externs(context, externs);
        for (j=0; j < 4; j++) {
            cod_parse_for_globals(global_decls[j], context);
        }
        cod_parse_for_context(extern_string, context);
        cod_subroutine_declaration(func_decls[i], context);
        gen_code[i] = cod_code_gen(func_bodies[i], context);
        externs[i].extern_value = (void*) gen_code[i]->func;
        if (i == 2) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/991201-1.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 991201-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/991201-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/991201-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/991201-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/991201-1.c Succeeded\n");
    return 0;
}