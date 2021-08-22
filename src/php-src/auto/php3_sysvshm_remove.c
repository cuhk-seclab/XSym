#include "stdio.h"
#include "phli.h"
void php3_sysvshm_remove(INTERNAL_FUNCTION_PARAMETERS);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/php3_sysvshm_remove/";
HashTable *ht;
HashTable *list;
HashTable *plist;
pval *return_value=phli_construct_pval_string("arg_1");
php3_sysvshm_remove(ht, return_value, list, plist);
}