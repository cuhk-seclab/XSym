void _php3_strip_tags(char*, int, int, char*);
#include <stdio.h>
#include "phli.h"
int main(int argc, char *argv[]) { 
	char *result_dir = "/data/phli/results/php_tag_find/";
	char* arg_0 = phli_create_char_array("arg_0");
	int* arg_1 = phli_create_int("arg_1");
    int* arg_2 = phli_create_int("arg_2");
	char* arg_3 = phli_create_char_array("arg_3");
	_php3_strip_tags(arg_0, *arg_1, *arg_2, arg_3);
    phli_print_string(arg_0,strlen(arg_0) ,result_dir);

}
