#include "stdio.h"
#include "phli.h"
void _php3_gettimeofday(pval*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/_php3_gettimeofday/";
pval* arg_0=phli_construct_pval_string("arg_0");
_php3_gettimeofday(arg_0);
}