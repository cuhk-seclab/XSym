#include "stdio.h"
#include "phli.h"
int send_getremotechildren(int,char*,char*,int*,int*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/send_getremotechildren/";
long* arg_0=phli_create_long("arg_0");
char* arg_1=phli_create_char_array("arg_1");
char* arg_2=phli_create_char_array("arg_2");
int* arg_3=phli_create_int_array("arg_3");
int* arg_4=phli_create_int_array("arg_4");
int result;
result=send_getremotechildren(*arg_0,arg_1,arg_2,arg_3,arg_4);
}