#include "stdio.h"
#include "phli.h"
void rpc_put_pval(char*,int*,int*,pval*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/rpc_put_pval/";
char* arg_0=phli_create_char_array("arg_0");
int* arg_1=phli_create_int_array("arg_1");
int* arg_2=phli_create_int_array("arg_2");
pval* arg_3=phli_construct_pval_string("arg_3");
rpc_put_pval(arg_0,arg_1,arg_2,arg_3);
}