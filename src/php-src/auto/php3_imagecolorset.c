#include "stdio.h"
#include "phli.h"
void php3_imagecolorset(INTERNAL_FUNCTION_PARAMETERS);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/php3_imagecolorset/";
HashTable *ht;
HashTable *list;
HashTable *plist;
pval *return_value=phli_construct_pval_string("arg_1");
php3_imagecolorset(ht, return_value, list, plist);
}