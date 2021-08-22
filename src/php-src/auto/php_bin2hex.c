#include "stdio.h"
#include "phli.h"
unsigned char* php_bin2hex(unsigned char*,size_t,size_t*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/php_bin2hex/";
unsigned char* arg_0=phli_create_unsigned_char_array("arg_0");
unsigned int* arg_1=phli_create_unsignedint("arg_1");
unsigned int* arg_2=phli_create_unsigned_int_array("arg_2");
unsigned char* result;
result=php_bin2hex(arg_0,*arg_1,arg_2);
}