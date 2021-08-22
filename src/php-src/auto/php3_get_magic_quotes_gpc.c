#include "stdio.h"
#include "phli.h"
void php3_get_magic_quotes_gpc(INTERNAL_FUNCTION_PARAMETERS);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/php3_get_magic_quotes_gpc/";
HashTable *ht;
HashTable *list;
HashTable *plist;
pval *return_value=phli_construct_pval_string("arg_1");
php3_get_magic_quotes_gpc(ht, return_value, list, plist);
}