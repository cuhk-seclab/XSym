#include "stdio.h"
#include "phli.h"
void mm_log(char*,long);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/mm_log/";
char* arg_0=phli_create_char_array("arg_0");
long* arg_1=phli_create_long("arg_1");
mm_log(arg_0,*arg_1);
}