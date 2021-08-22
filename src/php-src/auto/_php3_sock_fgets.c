#include "stdio.h"
#include "phli.h"
char* _php3_sock_fgets(char*,size_t,int);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/_php3_sock_fgets/";
char* arg_0=phli_create_char_array("arg_0");
unsigned int* arg_1=phli_create_unsignedint("arg_1");
long* arg_2=phli_create_long("arg_2");
char* result;
result=_php3_sock_fgets(arg_0,*arg_1,*arg_2);
}