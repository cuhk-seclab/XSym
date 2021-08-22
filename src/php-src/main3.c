#include <stdio.h>
#include "phli.h"
void _php3_trim(pval*, pval*);
int main(int argc, char *argv[]) { 
	char *result_dir = "/data/phli/results/_php3_trim/";
	pval* arg_0 = phli_construct_pval_string("arg_0");
	pval* arg_1 = phli_construct_pval_string("arg_1");
	_php3_trim(arg_0, arg_1);
    phli_print_string(arg_1->value.str.val, arg_1->value.str.len, result_dir);

}
