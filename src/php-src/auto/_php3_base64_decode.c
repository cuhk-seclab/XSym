#include "stdio.h"
#include "phli.h"
unsigned char* _php3_base64_decode(char*,int,int*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/_php3_base64_decode/";
char* arg_0=phli_create_char_array("arg_0");
long* arg_1=phli_create_long("arg_1");
int* arg_2=phli_create_int_array("arg_2");
unsigned char* result;
result=_php3_base64_decode(arg_0,*arg_1,arg_2);
}