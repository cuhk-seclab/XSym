#include "stdio.h"
#include "phli.h"
int initialize_hg_connection(int,int*,int*,char*,char*,char*,char*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/initialize_hg_connection/";
long* arg_0=phli_create_long("arg_0");
int* arg_1=phli_create_int_array("arg_1");
int* arg_2=phli_create_int_array("arg_2");
char* arg_3=phli_create_char_array("arg_3");
char* arg_4=phli_create_char_array("arg_4");
char* arg_5=phli_create_char_array("arg_5");
char* arg_6=phli_create_char_array("arg_6");
int result;
result=initialize_hg_connection(*arg_0,arg_1,arg_2,arg_3,arg_4,arg_5,arg_6);
}