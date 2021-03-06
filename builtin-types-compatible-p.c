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
// int i;
// double d;
// 
// /* Make sure we return a constant.  */
// float rootbeer[__builtin_types_compatible_p (int, typeof(i))];
// 
// typedef enum { hot, dog, poo, bear } dingos;
// typedef enum { janette, laura, amanda } cranberry;
// 
// typedef float same1;
// typedef float same2;
// 
// int main (void);
// 
// int main (void)
// {
//   /* Compatible types.  */
//   if (!(__builtin_types_compatible_p (int, const int)
// 	&& __builtin_types_compatible_p (typeof (hot), int)
// 	&& __builtin_types_compatible_p (typeof (hot), typeof (laura))
// 	&& __builtin_types_compatible_p (int[5], int[])
// 	&& __builtin_types_compatible_p (typeof (dingos), typeof (cranberry))
// 	&& __builtin_types_compatible_p (same1, same2)))
//     abort ();
// 
//   /* Incompatible types.  */
//   if (__builtin_types_compatible_p (char *, int)
//       || __builtin_types_compatible_p (char *, const char *)
//       || __builtin_types_compatible_p (long double, double)
//       || __builtin_types_compatible_p (typeof (i), typeof (d))
//       || __builtin_types_compatible_p (char, int)
//       || __builtin_types_compatible_p (char *, char **))
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
	test_output = fopen("builtin-types-compatible-p.c.output", "w");
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
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	int main ();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
	"int i;\n\
double d;\n\
\n\
/* Make sure we return a constant.  */\n\
float rootbeer[__builtin_types_compatible_p (int, typeof(i))];",
	"typedef enum { hot, dog, poo, bear } dingos;",
	"typedef enum { janette, laura, amanda } cranberry;\n\
\n\
typedef float same1;\n\
typedef float same2;\n\
\n\
int main (void);",
""};

    char *func_decls[] = {
	"int main ();",
	""};

    char *func_bodies[] = {

/* body for main */
"{\n\
  /* Compatible types.  */\n\
  if (!(__builtin_types_compatible_p (int, const int)\n\
	&& __builtin_types_compatible_p (typeof (hot), int)\n\
	&& __builtin_types_compatible_p (typeof (hot), typeof (laura))\n\
	&& __builtin_types_compatible_p (int[5], int[])\n\
	&& __builtin_types_compatible_p (typeof (dingos), typeof (cranberry))\n\
	&& __builtin_types_compatible_p (same1, same2)))\n\
    abort ();\n\
\n\
  /* Incompatible types.  */\n\
  if (__builtin_types_compatible_p (char *, int)\n\
      || __builtin_types_compatible_p (char *, const char *)\n\
      || __builtin_types_compatible_p (long double, double)\n\
      || __builtin_types_compatible_p (typeof (i), typeof (d))\n\
      || __builtin_types_compatible_p (char, int)\n\
      || __builtin_types_compatible_p (char *, char **))\n\
    abort ();\n\
  exit (0);\n\
}",
""};

    int i;
    cod_code gen_code[1];
    cod_parse_context context;
    for (i=0; i < 1; i++) {
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
        if (i == 0) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/builtin-types-compatible-p.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp builtin-types-compatible-p.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/builtin-types-compatible-p.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/builtin-types-compatible-p.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/builtin-types-compatible-p.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/builtin-types-compatible-p.c Succeeded\n");
    return 0;
}
