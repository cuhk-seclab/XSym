#include "stdio.h"
#include "phli.h"
int _php3_sock_destroy(int);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/_php3_sock_destroy/";
long* arg_0=phli_create_long("arg_0");
int result;
result=_php3_sock_destroy(*arg_0);
}