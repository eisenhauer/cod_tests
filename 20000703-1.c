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
// void abort(void);
// void exit(int);
// #include "string.h"
// 
// struct baz 
// {
//   char a[17];
//   char b[3];
//   unsigned int c;
//   unsigned int d;
// };
// 
// void foo(struct baz *p, unsigned int c, unsigned int d)
// {
//   memcpy (p->b, "abc", 3);
//   p->c = c;
//   p->d = d;
// }
// 
// void bar(struct baz *p, unsigned int c, unsigned int d)
// {
//   ({ void *s = (p);
//      __builtin_memset (s, '\0', sizeof (struct baz));
//      s; });
//   memcpy (p->a, "01234567890123456", 17);
//   memcpy (p->b, "abc", 3);
//   p->c = c;
//   p->d = d;
// }
// 
// int main()
// {
//   struct baz p;
//   foo(&p, 71, 18);
//   if (p.c != 71 || p.d != 18)
//     abort();
//   bar(&p, 59, 26);
//   if (p.c != 59 || p.d != 26)
//     abort();
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
	test_output = fopen("20000703-1.c.output", "w");
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
	{"foo", (void*)(long)-1},
	{"bar", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	void foo(struct baz *p, unsigned int c, unsigned int d);\n\
	void bar(struct baz *p, unsigned int c, unsigned int d);\n\
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
	"\n\
\n\
#include \"string.h\"",
	"struct baz{\n\
  char a[17];\n\
  char b[3];\n\
  unsigned int c;\n\
  unsigned int d;\n\
};",
""};

    char *func_decls[] = {
	"void foo(struct baz *p, unsigned int c, unsigned int d);",
	"void bar(struct baz *p, unsigned int c, unsigned int d);",
	"int main();",
	""};

    char *func_bodies[] = {

/* body for foo */
"{\n\
  memcpy (p->b, \"abc\", 3);\n\
  p->c = c;\n\
  p->d = d;\n\
}",

/* body for bar */
"{\n\
  ({ void *s = (p);\n\
     __builtin_memset (s, '\\0', sizeof (struct baz));\n\
     s; });\n\
  memcpy (p->a, \"01234567890123456\", 17);\n\
  memcpy (p->b, \"abc\", 3);\n\
  p->c = c;\n\
  p->d = d;\n\
}",

/* body for main */
"{\n\
  struct baz p;\n\
  foo(&p, 71, 18);\n\
  if (p.c != 71 || p.d != 18)\n\
    abort();\n\
  bar(&p, 59, 26);\n\
  if (p.c != 59 || p.d != 26)\n\
    abort();\n\
  exit(0);\n\
}",
""};

    int i;
    cod_code gen_code[3];
    cod_parse_context context;
    for (i=0; i < 3; i++) {
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
        if (i == 2) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/20000703-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20000703-1.c.output ./pre_patch/20000703-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20000703-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20000703-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20000703-1.c Succeeded\n");
    return 0;
}
