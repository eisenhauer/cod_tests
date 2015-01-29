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
	test_output = fopen("20020615-1.c.output", "w");
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
	{"line_hints", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	int line_hints(const font_hints *fh, const gs_fixed_point *p0,
	   const gs_fixed_point *p1);\n\
	int main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);";
    char *func_bodies[] = {

/* body for line_hints */
"{\n\
  long dx = p1->x - p0->x;\n\
  long dy = p1->y - p0->y;\n\
  long adx, ady;\n\
  int xi = fh->x_inverted, yi = fh->y_inverted;\n\
  int hints;\n\
  if (xi)\n\
    dx = -dx;\n\
  if (yi)\n\
    dy = -dy;\n\
  if (fh->axes_swapped) {\n\
    long t = dx;\n\
    int ti = xi;\n\
    dx = dy, xi = yi;\n\
    dy = t, yi = ti;\n\
  }\n\
  adx = dx < 0 ? -dx : dx;\n\
  ady = dy < 0 ? -dy : dy;\n\
  if (dy != 0 && (adx <= ady >> 4)) {\n\
    hints = dy > 0 ? 2 : 1;\n\
    if (xi)\n\
      hints ^= 3;\n\
  } else if (dx != 0 && (ady <= adx >> 4)) {\n\
    hints = dx < 0 ? 8 : 4;\n\
    if (yi)\n\
      hints ^= 12;\n\
  } else\n\
    hints = 0;\n\
  return hints;\n\
}",

/* body for main */
"{\n\
  static font_hints fh[] = {{0, 1, 0}, {0, 0, 1}, {0, 0, 0}};\n\
  static gs_fixed_point gsf[]\n\
    = {{0x30000, 0x13958}, {0x30000, 0x18189},\n\
       {0x13958, 0x30000}, {0x18189, 0x30000}};\n\
  if (line_hints (fh, gsf, gsf + 1) != 1\n\
      || line_hints (fh + 1, gsf + 2, gsf + 3) != 8\n\
      || line_hints (fh + 2, gsf + 2, gsf + 3) != 4)\n\
    abort ();\n\
  exit (0);\n\
}",
""};

    char *func_decls[] = {
	"int line_hints(const font_hints *fh, const gs_fixed_point *p0,
	   const gs_fixed_point *p1);",
	"int main ();",
	""};

    char *global_decls[] = {
	"typedef struct font_hints_s {\n\
  int axes_swapped;\n\
  int x_inverted, y_inverted;\n\
} font_hints;",
	"typedef struct gs_fixed_point_s {\n\
  long x, y;\n\
} gs_fixed_point;",
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
        for (j=0; j < 2; j++) {
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
                printf("Test ./generated/20020615-1.c failed\n");
                exit(exit_value);
            }
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20020615-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20020615-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20020615-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20020615-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20020615-1.c Succeeded\n");
    return 0;
}
