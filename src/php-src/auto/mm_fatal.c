#include "stdio.h"
#include "phli.h"
void mm_fatal(char*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/mm_fatal/";
char* arg_0=phli_create_char_array("arg_0");
mm_fatal(arg_0);
}