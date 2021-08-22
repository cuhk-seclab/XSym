#include "stdio.h"
#include "phli.h"
unsigned char* php_bin2hex(unsigned char*, size_t, size_t*);

int main(int argc, char *argv[]) { 
	char *result_dir = "/data/phli/results/php3_bin2hex/";
	unsigned char* arg_0 = phli_create_unsigned_char_array("arg_0");
	//size_t* arg_1=phli_create_size_t("arg_1");
    size_t *arg_1,*arg_2;
	unsigned char* phli_result;
	phli_result = php_bin2hex(arg_0, *arg_1, arg_2);
    phli_print_string((char*)phli_result, strlen((const char*)phli_result), result_dir);

}
