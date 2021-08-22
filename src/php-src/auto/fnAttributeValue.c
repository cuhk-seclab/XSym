#include "stdio.h"
#include "phli.h"
char* fnAttributeValue(char*,char*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/fnAttributeValue/";
char* arg_0=phli_create_char_array("arg_0");
char* arg_1=phli_create_char_array("arg_1");
char* result;
result=fnAttributeValue(arg_0,arg_1);
}