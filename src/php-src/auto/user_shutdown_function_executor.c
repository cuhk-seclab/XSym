#include "stdio.h"
#include "phli.h"
int user_shutdown_function_executor(pval*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/user_shutdown_function_executor/";
pval* arg_0=phli_construct_pval_string("arg_0");
int result;
result=user_shutdown_function_executor(arg_0);
}