#include "stdio.h"
#include "phli.h"
char* strerror(int);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/strerror/";
long* arg_0=phli_create_long("arg_0");
char* result;
result=strerror(*arg_0);
}