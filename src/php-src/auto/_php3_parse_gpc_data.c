#include "stdio.h"
#include "phli.h"
void _php3_parse_gpc_data(char*,char*,pval*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/_php3_parse_gpc_data/";
char* arg_0=phli_create_char_array("arg_0");
char* arg_1=phli_create_char_array("arg_1");
pval* arg_2=phli_construct_pval_string("arg_2");
_php3_parse_gpc_data(arg_0,arg_1,arg_2);
}