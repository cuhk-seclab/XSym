#include "stdio.h"
#include "phli.h"
int send_edittext(int,char*,char*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/send_edittext/";
long* arg_0=phli_create_long("arg_0");
char* arg_1=phli_create_char_array("arg_1");
char* arg_2=phli_create_char_array("arg_2");
int result;
result=send_edittext(*arg_0,arg_1,arg_2);
}