#include "stdio.h"
#include "phli.h"
int _php3_mail(char*,char*,char*,char*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/_php3_mail/";
char* arg_0=phli_create_char_array("arg_0");
char* arg_1=phli_create_char_array("arg_1");
char* arg_2=phli_create_char_array("arg_2");
char* arg_3=phli_create_char_array("arg_3");
int result;
result=_php3_mail(arg_0,arg_1,arg_2,arg_3);
}