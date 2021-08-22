#include "stdio.h"
#include "phli.h"
void php3api_var_dump(pval*,int);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/php3api_var_dump/";
pval* arg_0=phli_construct_pval_string("arg_0");
long* arg_1=phli_create_long("arg_1");
php3api_var_dump(arg_0,*arg_1);
}