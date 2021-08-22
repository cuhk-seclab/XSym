#include "stdio.h"
#include "phli.h"
char* fnInsStr(char*,int,char*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/fnInsStr/";
char* arg_0=phli_create_char_array("arg_0");
long* arg_1=phli_create_long("arg_1");
char* arg_2=phli_create_char_array("arg_2");
char* result;
result=fnInsStr(arg_0,*arg_1,arg_2);
}