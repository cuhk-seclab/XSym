#include "stdio.h"
#include "phli.h"
int fn_findpath(int,int*,int,int);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/fn_findpath/";
long* arg_0=phli_create_long("arg_0");
int* arg_1=phli_create_int_array("arg_1");
long* arg_2=phli_create_long("arg_2");
long* arg_3=phli_create_long("arg_3");
int result;
result=fn_findpath(*arg_0,arg_1,*arg_2,*arg_3);
}