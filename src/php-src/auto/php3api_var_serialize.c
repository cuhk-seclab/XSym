#include "stdio.h"
#include "phli.h"
void php3api_var_serialize(pval*,pval*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/php3api_var_serialize/";
pval* arg_0=phli_construct_pval_string("arg_0");
pval* arg_1=phli_construct_pval_string("arg_1");
php3api_var_serialize(arg_0,arg_1);
}