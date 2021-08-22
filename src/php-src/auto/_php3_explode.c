#include "stdio.h"
#include "phli.h"
void _php3_explode(pval*,pval*,pval*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/_php3_explode/";
pval* arg_0=phli_construct_pval_string("arg_0");
pval* arg_1=phli_construct_pval_string("arg_1");
pval* arg_2=phli_construct_pval_string("arg_2");
_php3_explode(arg_0,arg_1,arg_2);
}