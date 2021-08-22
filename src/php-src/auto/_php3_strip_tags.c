#include "stdio.h"
#include "phli.h"
void _php3_strip_tags(char*,int,int,char*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/_php3_strip_tags/";
char* arg_0=phli_create_char_array("arg_0");
long* arg_1=phli_create_long("arg_1");
long* arg_2=phli_create_long("arg_2");
char* arg_3=phli_create_char_array("arg_3");
_php3_strip_tags(arg_0,*arg_1,*arg_2,arg_3);
}