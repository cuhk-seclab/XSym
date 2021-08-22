#include <stdio.h>
#include "phli.h"
void php3_ord1(INTERNAL_FUNCTION_PARAMETERS);
int main(int argc, char *argv[]) { 
	char *result_dir = "/data/phli/results/tmp/";
    HashTable *ht;
    pval *return_value = phli_construct_pval_string("arg_1");
    HashTable *list;
    HashTable *plist;
	php3_ord1(ht, return_value, list, plist);
    phli_print_long(&return_value->value.lval, result_dir);

}
