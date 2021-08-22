#include "stdio.h"
#include "phli.h"
int send_getdestforanchorsobj(int,char*,char*,int);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/send_getdestforanchorsobj/";
long* arg_0=phli_create_long("arg_0");
char* arg_1=phli_create_char_array("arg_1");
char* arg_2=phli_create_char_array("arg_2");
long* arg_3=phli_create_long("arg_3");
int result;
result=send_getdestforanchorsobj(*arg_0,arg_1,arg_2,*arg_3);
}