#include "stdio.h"
#include "phli.h"
int send_ready(int);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/send_ready/";
long* arg_0=phli_create_long("arg_0");
int result;
result=send_ready(*arg_0);
}