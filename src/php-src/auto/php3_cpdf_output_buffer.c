#include "stdio.h"
#include "phli.h"
void php3_cpdf_output_buffer(INTERNAL_FUNCTION_PARAMETERS);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/php3_cpdf_output_buffer/";
HashTable *ht;
HashTable *list;
HashTable *plist;
pval *return_value=phli_construct_pval_string("arg_1");
php3_cpdf_output_buffer(ht, return_value, list, plist);
}