#include "stdio.h"
#include "phli.h"
int open_hg_connection(char*,int);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/open_hg_connection/";
char* arg_0=phli_create_char_array("arg_0");
long* arg_1=phli_create_long("arg_1");
int result;
result=open_hg_connection(arg_0,*arg_1);
}