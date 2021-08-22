#include "stdio.h"
#include "phli.h"
int fnAttributeCompare(char*,char*,char*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/fnAttributeCompare/";
char* arg_0=phli_create_char_array("arg_0");
char* arg_1=phli_create_char_array("arg_1");
char* arg_2=phli_create_char_array("arg_2");
int result;
result=fnAttributeCompare(arg_0,arg_1,arg_2);
}