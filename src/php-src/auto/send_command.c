#include "stdio.h"
#include "phli.h"
int send_command(int,int,char*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/send_command/";
long* arg_0=phli_create_long("arg_0");
long* arg_1=phli_create_long("arg_1");
char* arg_2=phli_create_char_array("arg_2");
int result;
result=send_command(*arg_0,*arg_1,arg_2);
}