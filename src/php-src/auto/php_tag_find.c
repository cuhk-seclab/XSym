#include "stdio.h"
#include "phli.h"
int php_tag_find(char*,int,char*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/php_tag_find/";
char* arg_0=phli_create_char_array("arg_0");
long* arg_1=phli_create_long("arg_1");
char* arg_2=phli_create_char_array("arg_2");
int result;
result=php_tag_find(arg_0,*arg_1,arg_2);
}