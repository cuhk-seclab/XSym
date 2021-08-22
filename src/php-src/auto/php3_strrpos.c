#include "stdio.h"
#include "phli.h"
void php3_strrpos(pval*,pval*,pval*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/php3_strrpos/";
pval* arg_0=phli_construct_pval_string("arg_0");
pval* arg_1=phli_construct_pval_string("arg_1");
pval* arg_2=phli_construct_pval_string("arg_2");
php3_strrpos(arg_0,arg_1,arg_2);
}