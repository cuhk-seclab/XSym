#include "stdio.h"
#include "phli.h"
void mm_bboard(char*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/mm_bboard/";
char* arg_0=phli_create_char_array("arg_0");
mm_bboard(arg_0);
}