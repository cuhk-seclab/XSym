#include "stdio.h"
#include "phli.h"
int php3api_var_unserialize(pval*,char*,char*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/php3api_var_unserialize/";
pval* arg_0=phli_construct_pval_string("arg_0");
char* arg_1=phli_create_char_array("arg_1");
char* arg_2=phli_create_char_array("arg_2");
int result;
result=php3api_var_unserialize(arg_0,arg_1,arg_2);
}