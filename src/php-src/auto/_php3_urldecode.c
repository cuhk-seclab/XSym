#include "stdio.h"
#include "phli.h"
int _php3_urldecode(char*,int);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/_php3_urldecode/";
char* arg_0=phli_create_char_array("arg_0");
long* arg_1=phli_create_long("arg_1");
int result;
result=_php3_urldecode(arg_0,*arg_1);
}