#include "stdio.h"
#include "phli.h"
char* _php3_urlencode(char*,int);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/_php3_urlencode/";
char* arg_0=phli_create_char_array("arg_0");
long* arg_1=phli_create_long("arg_1");
char* result;
result=_php3_urlencode(arg_0,*arg_1);
}