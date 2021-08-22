#include "stdio.h"
#include "phli.h"
int add_next_index_object(pval*,pval);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/add_next_index_object/";
pval* arg_0=phli_construct_pval_string("arg_0");
pval* arg_1=phli_construct_pval_string("arg_1");
int result;
result=add_next_index_object(arg_0,*arg_1);
}