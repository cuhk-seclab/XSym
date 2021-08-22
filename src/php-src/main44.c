#include <stdio.h>
#include "phli.h"
void php3_checkdate(INTERNAL_FUNCTION_PARAMETERS);
int main(int argc, char *argv[]) { 
	char *result_dir = "/data/phli/results/php3_checkdate";
    HashTable *ht;
    pval *return_value = phli_construct_pval_bool("arg_1");
    HashTable *list;
    HashTable *plist;
    php3_checkdate(ht, return_value, list, plist);
    phli_print_bool(&return_value->value.blval,  result_dir);

}
