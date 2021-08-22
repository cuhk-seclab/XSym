#include "stdio.h"
#include "phli.h"
int make_return_array_from_objrec(pval*,char*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/make_return_array_from_objrec/";
pval* arg_0=phli_construct_pval_string("arg_0");
char* arg_1=phli_create_char_array("arg_1");
int result;
result=make_return_array_from_objrec(arg_0,arg_1);
}