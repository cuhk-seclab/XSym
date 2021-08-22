#include <stdio.h>
#include <phli.h>
void php3_str_replace(pval *, pval *, pval*, pval*);
int main(int argc, char *argv[]) { 
    pval *arg_0 = phli_construct_pval_string("arg_0");
    pval *arg_1 = phli_construct_pval_string("arg_1");
    pval *arg_2 = phli_construct_pval_string("arg_2");
    pval *arg_3 = phli_construct_pval_string("arg_3");
	char *result_dir = "/data/phli/results/php3_str_replace/";
	php3_str_replace(arg_0, arg_1, arg_2, arg_3);
    phli_print_string(arg_3->value.str.val, arg_3->value.str.len, result_dir);

}
