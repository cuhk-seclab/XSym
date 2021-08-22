#include "stdio.h"
#include "phli.h"
void php3_hebrev_with_conversion(INTERNAL_FUNCTION_PARAMETERS);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/php3_hebrev_with_conversion/";
HashTable *ht;
HashTable *list;
HashTable *plist;
pval *return_value=phli_construct_pval_string("arg_1");
php3_hebrev_with_conversion(ht, return_value, list, plist);
}