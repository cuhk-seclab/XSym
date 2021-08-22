#include "stdio.h"
#include "phli.h"
int _php3_sock_set_blocking(int,int);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/_php3_sock_set_blocking/";
long* arg_0=phli_create_long("arg_0");
long* arg_1=phli_create_long("arg_1");
int result;
result=_php3_sock_set_blocking(*arg_0,*arg_1);
}