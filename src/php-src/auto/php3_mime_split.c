#include "stdio.h"
#include "phli.h"
void php3_mime_split(char*,int,char*,pval*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/php3_mime_split/";
char* arg_0=phli_create_char_array("arg_0");
long* arg_1=phli_create_long("arg_1");
char* arg_2=phli_create_char_array("arg_2");
pval* arg_3=phli_construct_pval_string("arg_3");
php3_mime_split(arg_0,*arg_1,arg_2,arg_3);
}