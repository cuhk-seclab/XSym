#include "stdio.h"
#include "phli.h"
void php3_treat_data(int,char*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/php3_treat_data/";
long* arg_0=phli_create_long("arg_0");
char* arg_1=phli_create_char_array("arg_1");
php3_treat_data(*arg_0,arg_1);
}