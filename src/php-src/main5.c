#include <stdio.h>
#include "phli.h"
char* _php3_strtoupper(char*);
int main(int argc, char *argv[]) { 
	char *result_dir = "/data/phli/results/_php3_strtoupper/";
	char* arg_0 = phli_create_char_array("arg_0");
	char* phli_result;
	phli_result = _php3_strtoupper(arg_0);
    phli_print_string(phli_result, strlen(phli_result), result_dir);

}
