#include <stdio.h>
#include "phli.h"
void php3_strrchr(INTERNAL_FUNCTION_PARAMETERS);
int main(int argc, char *argv[]) { 
	char *result_dir = "/data/phli/results/php3_strrchr/";
    HashTable *ht;
    pval *return_value = phli_construct_pval_string("arg_1");
    HashTable *list;
    HashTable *plist;
    php3_strrchr(ht, return_value, list, plist);
    phli_print_string(return_value->value.str.val, return_value->value.str.len, result_dir);

}
