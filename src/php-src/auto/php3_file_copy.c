#include "stdio.h"
#include "phli.h"
void php3_file_copy(INTERNAL_FUNCTION_PARAMETERS);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/php3_file_copy/";
HashTable *ht;
HashTable *list;
HashTable *plist;
pval *return_value=phli_construct_pval_string("arg_1");
php3_file_copy(ht, return_value, list, plist);
}