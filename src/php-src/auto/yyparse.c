#include "stdio.h"
#include "phli.h"
int yyparse(void);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/yyparse/";
int result;
result=yyparse();
}