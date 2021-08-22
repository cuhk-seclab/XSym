#include "stdio.h"
#include "phli.h"
void array_current_key(INTERNAL_FUNCTION_PARAMETERS);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/array_current_key/";
HashTable *ht;
HashTable *list;
HashTable *plist;
pval *return_value=phli_construct_pval_string("arg_1");
array_current_key(ht, return_value, list, plist);
}