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
	test_output = fopen("20021010-2.c.output", "w");
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
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	void main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for main */
"{\n\
  unsigned int *vramPtr;\n\
  int width;\n\
  IOGBounds saveRect = global_saveRect;\n\
  IOGBounds bounds = global_bounds;\n\
\n\
  if (saveRect.minx < bounds.minx) saveRect.minx = bounds.minx;\n\
  if (saveRect.maxx > bounds.maxx) saveRect.maxx = bounds.maxx;\n\
\n\
  vramPtr = global_vramPtr + (saveRect.miny - bounds.miny) ;\n\
  width = saveRect.maxx - saveRect.minx;\n\
\n\
  if (width != expectedwidth)\n\
    abort ();\n\
  exit (0);\n\
}",
""};

    char *func_decls[] = {
	"void main();",
	""};

    char *global_decls[] = {
	"#include <stdlib.h>\n\
\n\
typedef signed short SInt16;",
	"typedef struct {\n\
    SInt16 minx;\n\
    SInt16 maxx;\n\
    SInt16 miny;\n\
    SInt16 maxy;\n\
} IOGBounds;\n\
\n\
int expectedwidth = 50;\n\
\n\
unsigned int *global_vramPtr = (unsigned int *)0xa000;",
	"IOGBounds global_bounds =",
	";\n\
IOGBounds global_saveRect =",
	";",
""};

    int i;
    cod_code gen_code[1];
    for (i=0; i < 1; i++) {
        int j;
        if (verbose) {
             printf("Working on subroutine %s\n", externs[i].extern_name);
        }
        cod_parse_context context = new_cod_parse_context();
        cod_assoc_externs(context, externs);
        for (j=0; j < 5; j++) {
            cod_parse_for_globals(global_decls[j], context);
        }
        cod_parse_for_context(extern_string, context);
        cod_subroutine_declaration(func_decls[i], context);
        gen_code[i] = cod_code_gen(func_bodies[i], context);
        externs[i].extern_value = (void*) gen_code[i]->func;
        if (i == 0) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/20021010-2.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20021010-2.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20021010-2.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20021010-2.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20021010-2.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20021010-2.c Succeeded\n");
    return 0;
}
