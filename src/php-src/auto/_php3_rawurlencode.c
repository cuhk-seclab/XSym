#include "stdio.h"
#include "phli.h"
char* _php3_rawurlencode(char*,int);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/_php3_rawurlencode/";
char* arg_0=phli_create_char_array("arg_0");
long* arg_1=phli_create_long("arg_1");
char* result;
result=_php3_rawurlencode(arg_0,*arg_1);
}