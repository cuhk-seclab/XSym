#include "stdio.h"
#include "phli.h"
int getrellink(int,int,int,int,char*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/getrellink/";
long* arg_0=phli_create_long("arg_0");
long* arg_1=phli_create_long("arg_1");
long* arg_2=phli_create_long("arg_2");
long* arg_3=phli_create_long("arg_3");
char* arg_4=phli_create_char_array("arg_4");
int result;
result=getrellink(*arg_0,*arg_1,*arg_2,*arg_3,arg_4);
}