#include "stdio.h"
#include "phli.h"
void php3_mysql_errno(INTERNAL_FUNCTION_PARAMETERS);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/php3_mysql_errno/";
HashTable *ht;
HashTable *list;
HashTable *plist;
pval *return_value=phli_construct_pval_string("arg_1");
php3_mysql_errno(ht, return_value, list, plist);
}