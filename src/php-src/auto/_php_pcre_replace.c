#include "stdio.h"
#include "phli.h"
char* _php_pcre_replace(char*,char*,char*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/_php_pcre_replace/";
char* arg_0=phli_create_char_array("arg_0");
char* arg_1=phli_create_char_array("arg_1");
char* arg_2=phli_create_char_array("arg_2");
char* result;
result=_php_pcre_replace(arg_0,arg_1,arg_2);
}