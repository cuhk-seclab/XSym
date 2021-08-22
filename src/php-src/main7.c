char* _php3_strtr(char*, int, char*, char*, int);
#include <stdio.h>
#include "phli.h"
int main(int argc, char *argv[]) { 
	char *result_dir = "/data/phli/results/_php3_strtr/";
	char* arg_0 = phli_create_char_array("arg_0");
	int* arg_1 = phli_create_int("arg_1");
	char* arg_2 = phli_create_char_array("arg_2");
	char* arg_3 = phli_create_char_array("arg_3");
	int* arg_4 = phli_create_int("arg_4");
	char* phli_result;
	phli_result = _php3_strtr(arg_0, *arg_1, arg_2, arg_3, *arg_4);
    phli_print_string(phli_result, strlen(phli_result), result_dir);

}
