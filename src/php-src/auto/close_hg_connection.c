#include "stdio.h"
#include "phli.h"
void close_hg_connection(int);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/close_hg_connection/";
long* arg_0=phli_create_long("arg_0");
close_hg_connection(*arg_0);
}