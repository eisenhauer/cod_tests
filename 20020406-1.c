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
// // Origin: abbott@dima.unige.it
// // PR c/5120
// 
// extern void * malloc (__SIZE_TYPE__);
// extern void * calloc (__SIZE_TYPE__, __SIZE_TYPE__);
// 
// typedef unsigned int FFelem;
// 
// FFelem FFmul(const FFelem x, const FFelem y)
// {
//   return x;
// }
// 
// 
// struct DUPFFstruct
// {
//   int maxdeg;
//   int deg;
//   FFelem *coeffs;
// };
// 
// typedef struct DUPFFstruct *DUPFF;
// 
// 
// int DUPFFdeg(const DUPFF f)
// {
//   return f->deg;
// }
// 
// 
// DUPFF DUPFFnew(const int maxdeg)
// {
//   DUPFF ans = (DUPFF)malloc(sizeof(struct DUPFFstruct));
//   ans->coeffs = 0;
//   if (maxdeg >= 0) ans->coeffs = (FFelem*)calloc(maxdeg+1,sizeof(FFelem));
//   ans->maxdeg = maxdeg;
//   ans->deg = -1;
//   return ans;
// }
// 
// void DUPFFfree(DUPFF x)
// {
// }
// 
// void DUPFFswap(DUPFF x, DUPFF y)
// {
// }
// 
// 
// DUPFF DUPFFcopy(const DUPFF x)
// {
//   return x;
// }
// 
// 
// void DUPFFshift_add(DUPFF f, const DUPFF g, int deg, const FFelem coeff)
// {
// }
// 
// 
// DUPFF DUPFFexgcd(DUPFF *fcofac, DUPFF *gcofac, const DUPFF f, const DUPFF g)
// {
//   DUPFF u, v, uf, ug, vf, vg;
//   FFelem q, lcu, lcvrecip, p;
//   int df, dg, du, dv;
// 
//   printf("DUPFFexgcd called on degrees %d and %d\n", DUPFFdeg(f), DUPFFdeg(g));
//   if (DUPFFdeg(f) < DUPFFdeg(g)) return DUPFFexgcd(gcofac, fcofac, g, f);  /*** BUG IN THIS LINE ***/
//   if (DUPFFdeg(f) != 2 || DUPFFdeg(g) != 1) abort();
//   if (f->coeffs[0] == 0) return f;
//   /****** NEVER REACH HERE IN THE EXAMPLE ******/
//   p = 2;
// 
//   df = DUPFFdeg(f);  if (df < 0) df = 0; /* both inputs are zero */
//   dg = DUPFFdeg(g);  if (dg < 0) dg = 0; /* one input is zero */
//   u = DUPFFcopy(f);
//   v = DUPFFcopy(g);
// 
//   uf = DUPFFnew(dg); uf->coeffs[0] = 1; uf->deg = 0;
//   ug = DUPFFnew(df);
//   vf = DUPFFnew(dg);
//   vg = DUPFFnew(df); vg->coeffs[0] = 1; vg->deg = 0;
// 
//   while (DUPFFdeg(v) > 0)
//   {
//     dv = DUPFFdeg(v);
//     lcvrecip = FFmul(1, v->coeffs[dv]);
//     while (DUPFFdeg(u) >= dv)
//     {
//       du = DUPFFdeg(u);
//       lcu = u->coeffs[du];
//       q = FFmul(lcu, lcvrecip);
//       DUPFFshift_add(u, v, du-dv, p-q);
//       DUPFFshift_add(uf, vf, du-dv, p-q);
//       DUPFFshift_add(ug, vg, du-dv, p-q);
//     }
//     DUPFFswap(u, v);
//     DUPFFswap(uf, vf);
//     DUPFFswap(ug, vg);
//   }
//   if (DUPFFdeg(v) == 0)
//   {
//     DUPFFswap(u, v);
//     DUPFFswap(uf, vf);
//     DUPFFswap(ug, vg);
//   }
//   DUPFFfree(vf);
//   DUPFFfree(vg);
//   DUPFFfree(v);
//   *fcofac = uf;
//   *gcofac = ug;
//   return u;
// }
// 
// 
// 
// int main()
// {
//   DUPFF f, g, cf, cg, h;
//   f = DUPFFnew(1); f->coeffs[1] = 1; f->deg = 1;
//   g = DUPFFnew(2); g->coeffs[2] = 1; g->deg = 2;
// 
//   printf("calling DUPFFexgcd on degrees %d and %d\n", DUPFFdeg(f), DUPFFdeg(g)) ;
//   h = DUPFFexgcd(&cf, &cg, f, g);
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
	test_output = fopen("20020406-1.c.output", "w");
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
	{"DUPFFdeg", (void*)(long)-1},
	{"DUPFFnew", (void*)(long)-1},
	{"DUPFFfree", (void*)(long)-1},
	{"DUPFFswap", (void*)(long)-1},
	{"DUPFFcopy", (void*)(long)-1},
	{"DUPFFshift_add", (void*)(long)-1},
	{"DUPFFexgcd", (void*)(long)-1},
	{"main", (void*)(long)-1},
	{"abort", (void*)my_abort},
	{"exit", (void*)test_exit},
	{"test_printf", (void*)test_printf},
	{"printf", (void*)printf},
	{(void*)0, (void*)0}
    };

    char extern_string[] = "\n\
	int DUPFFdeg(const DUPFF f);\n\
	DUPFF DUPFFnew(const int maxdeg);\n\
	void DUPFFfree(DUPFF x);\n\
	void DUPFFswap(DUPFF x, DUPFF y);\n\
	DUPFF DUPFFcopy(const DUPFF x);\n\
	void DUPFFshift_add(DUPFF f, const DUPFF g, int deg, const FFelem coeff);\n\
	DUPFF DUPFFexgcd(DUPFF *fcofac, DUPFF *gcofac, const DUPFF f, const DUPFF g);\n\
	int main();\n\
    	void exit(int value);\n\
        void abort();\n\
        int test_printf(const char *format, ...);\n\
        int printf(const char *format, ...);";
    char *global_decls[] = {
	"struct DUPFFstruct\n\
{\n\
  int maxdeg;\n\
  int deg;\n\
  FFelem *coeffs;\n\
};\n\
\n\
typedef struct DUPFFstruct *DUPFF;",
""};

    char *func_decls[] = {
	"int DUPFFdeg(const DUPFF f);",
	"DUPFF DUPFFnew(const int maxdeg);",
	"void DUPFFfree(DUPFF x);",
	"void DUPFFswap(DUPFF x, DUPFF y);",
	"DUPFF DUPFFcopy(const DUPFF x);",
	"void DUPFFshift_add(DUPFF f, const DUPFF g, int deg, const FFelem coeff);",
	"DUPFF DUPFFexgcd(DUPFF *fcofac, DUPFF *gcofac, const DUPFF f, const DUPFF g);",
	"int main();",
	""};

    char *func_bodies[] = {

/* body for DUPFFdeg */
"{\n\
  return f->deg;\n\
}",

/* body for DUPFFnew */
"{\n\
  DUPFF ans = (DUPFF)malloc(sizeof(struct DUPFFstruct));\n\
  ans->coeffs = 0;\n\
  if (maxdeg >= 0) ans->coeffs = (FFelem*)calloc(maxdeg+1,sizeof(FFelem));\n\
  ans->maxdeg = maxdeg;\n\
  ans->deg = -1;\n\
  return ans;\n\
}",

/* body for DUPFFfree */
"{\n\
}",

/* body for DUPFFswap */
"{\n\
}",

/* body for DUPFFcopy */
"{\n\
  return x;\n\
}",

/* body for DUPFFshift_add */
"{\n\
}",

/* body for DUPFFexgcd */
"{\n\
  DUPFF u, v, uf, ug, vf, vg;\n\
  FFelem q, lcu, lcvrecip, p;\n\
  int df, dg, du, dv;\n\
\n\
  test_printf(\"DUPFFexgcd called on degrees %d and %d\\n\", DUPFFdeg(f), DUPFFdeg(g));\n\
  if (DUPFFdeg(f) < DUPFFdeg(g)) return DUPFFexgcd(gcofac, fcofac, g, f);  /*** BUG IN THIS LINE ***/\n\
  if (DUPFFdeg(f) != 2 || DUPFFdeg(g) != 1) abort();\n\
  if (f->coeffs[0] == 0) return f;\n\
  /****** NEVER REACH HERE IN THE EXAMPLE ******/\n\
  p = 2;\n\
\n\
  df = DUPFFdeg(f);  if (df < 0) df = 0; /* both inputs are zero */\n\
  dg = DUPFFdeg(g);  if (dg < 0) dg = 0; /* one input is zero */\n\
  u = DUPFFcopy(f);\n\
  v = DUPFFcopy(g);\n\
\n\
  uf = DUPFFnew(dg); uf->coeffs[0] = 1; uf->deg = 0;\n\
  ug = DUPFFnew(df);\n\
  vf = DUPFFnew(dg);\n\
  vg = DUPFFnew(df); vg->coeffs[0] = 1; vg->deg = 0;\n\
\n\
  while (DUPFFdeg(v) > 0)\n\
  {\n\
    dv = DUPFFdeg(v);\n\
    lcvrecip = FFmul(1, v->coeffs[dv]);\n\
    while (DUPFFdeg(u) >= dv)\n\
    {\n\
      du = DUPFFdeg(u);\n\
      lcu = u->coeffs[du];\n\
      q = FFmul(lcu, lcvrecip);\n\
      DUPFFshift_add(u, v, du-dv, p-q);\n\
      DUPFFshift_add(uf, vf, du-dv, p-q);\n\
      DUPFFshift_add(ug, vg, du-dv, p-q);\n\
    }\n\
    DUPFFswap(u, v);\n\
    DUPFFswap(uf, vf);\n\
    DUPFFswap(ug, vg);\n\
  }\n\
  if (DUPFFdeg(v) == 0)\n\
  {\n\
    DUPFFswap(u, v);\n\
    DUPFFswap(uf, vf);\n\
    DUPFFswap(ug, vg);\n\
  }\n\
  DUPFFfree(vf);\n\
  DUPFFfree(vg);\n\
  DUPFFfree(v);\n\
  *fcofac = uf;\n\
  *gcofac = ug;\n\
  return u;\n\
}",

/* body for main */
"{\n\
  DUPFF f, g, cf, cg, h;\n\
  f = DUPFFnew(1); f->coeffs[1] = 1; f->deg = 1;\n\
  g = DUPFFnew(2); g->coeffs[2] = 1; g->deg = 2;\n\
\n\
  test_printf(\"calling DUPFFexgcd on degrees %d and %d\\n\", DUPFFdeg(f), DUPFFdeg(g)) ;\n\
  h = DUPFFexgcd(&cf, &cg, f, g);\n\
  return 0;\n\
}",
""};

    int i;
    cod_code gen_code[8];
    cod_parse_context context;
    for (i=0; i < 8; i++) {
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
        if (i == 7) {
            int (*func)() = (int(*)()) externs[i].extern_value;
            if (setjmp(env) == 0) {
                func();
            }
            if (exit_value != 0) {
                printf("Test ./generated/20020406-1.c failed\n");
                exit(exit_value);
            }
        } else {
            context = cod_copy_globals(context);
        }
    }
    if (test_output) {
        /* there was output, test expected */
        fclose(test_output);
        int ret = system("cmp 20020406-1.c.output /Users/eisen/prog/gcc-3.3.1-3/gcc/testsuite/gcc.expect-torture/execute/20020406-1.expect");
        ret = ret >> 8;
        if (ret == 1) {
            printf("Test ./generated/20020406-1.c failed, output differs\n");
            exit(1);
        }
        if (ret != 0) {
            printf("Test ./generated/20020406-1.c failed, output missing\n");
            exit(1);
        }
    }
    if (verbose) printf("Test ./generated/20020406-1.c Succeeded\n");
    return 0;
}
