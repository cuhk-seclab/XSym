#include "stdio.h"
#include "phli.h"
int send_getobjbyqueryobj(int,char*,int,char*,int*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/send_getobjbyqueryobj/";
long* arg_0=phli_create_long("arg_0");
char* arg_1=phli_create_char_array("arg_1");
long* arg_2=phli_create_long("arg_2");
char* arg_3=phli_create_char_array("arg_3");
int* arg_4=phli_create_int_array("arg_4");
int result;
result=send_getobjbyqueryobj(*arg_0,arg_1,*arg_2,arg_3,arg_4);
}