#include "stdio.h"
#include "phli.h"
int php3_check_type(char*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/php3_check_type/";
char* arg_0=phli_create_char_array("arg_0");
int result;
result=php3_check_type(arg_0);
}