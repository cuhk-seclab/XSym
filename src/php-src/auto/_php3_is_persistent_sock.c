#include "stdio.h"
#include "phli.h"
int _php3_is_persistent_sock(int);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/_php3_is_persistent_sock/";
long* arg_0=phli_create_long("arg_0");
int result;
result=_php3_is_persistent_sock(*arg_0);
}