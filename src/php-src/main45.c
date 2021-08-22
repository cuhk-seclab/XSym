#include <stdio.h>
#include "phli.h"
void php3_opendir(INTERNAL_FUNCTION_PARAMETERS);
int main(int argc, char *argv[]) { 
	char *result_dir = "/data/phli/results/php3_opendir";
    HashTable *ht;
    pval *return_value = phli_construct_pval_long("arg_1");
    HashTable *list;
    HashTable *plist;
    php3_opendir(ht, return_value, list, plist);
    phli_print_long(&return_value->value.lval,  result_dir);

}
