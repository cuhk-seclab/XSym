#include "stdio.h"
#include "phli.h"
int send_getreldestforanchorsobj(int,char*,char*,int,int,int);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/send_getreldestforanchorsobj/";
long* arg_0=phli_create_long("arg_0");
char* arg_1=phli_create_char_array("arg_1");
char* arg_2=phli_create_char_array("arg_2");
long* arg_3=phli_create_long("arg_3");
long* arg_4=phli_create_long("arg_4");
long* arg_5=phli_create_long("arg_5");
int result;
result=send_getreldestforanchorsobj(*arg_0,arg_1,arg_2,*arg_3,*arg_4,*arg_5);
}