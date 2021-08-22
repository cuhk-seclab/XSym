#include "stdio.h"
#include "phli.h"
int read_to(int,char*,int,int);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/read_to/";
long* arg_0=phli_create_long("arg_0");
char* arg_1=phli_create_char_array("arg_1");
long* arg_2=phli_create_long("arg_2");
long* arg_3=phli_create_long("arg_3");
int result;
result=read_to(*arg_0,arg_1,*arg_2,*arg_3);
}