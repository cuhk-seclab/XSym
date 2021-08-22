#include "stdio.h"
#include "phli.h"
void user_shutdown_function_dtor(pval*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/user_shutdown_function_dtor/";
pval* arg_0=phli_construct_pval_string("arg_0");
user_shutdown_function_dtor(arg_0);
}