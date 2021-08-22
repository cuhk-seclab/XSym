#include "stdio.h"
#include "phli.h"
int add_assoc_object(pval*,char*,pval);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/add_assoc_object/";
pval* arg_0=phli_construct_pval_string("arg_0");
char* arg_1=phli_create_char_array("arg_1");
pval* arg_2=phli_construct_pval_string("arg_2");
int result;
result=add_assoc_object(arg_0,arg_1,*arg_2);
}