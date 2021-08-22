#include "stdio.h"
#include "phli.h"
void cc_login(char*,char*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/cc_login/";
char* arg_0=phli_create_char_array("arg_0");
char* arg_1=phli_create_char_array("arg_1");
cc_login(arg_0,arg_1);
}