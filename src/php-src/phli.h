#include <stdio.h>
#include "php.h"
#include "internal_functions.h"
#include "language-parser.tab.h"
#include "main.h"
#include "control_structures.h"
#include "modules.h"
#include "fopen-wrappers.h"
#include "functions/basic_functions.h"
#include "functions/info.h"
#include "functions/head.h"
#include "functions/post.h"
#include "functions/head.h"
#include "functions/type.h"
#include "highlight.h"
#include "php3_list.h"
#include "snprintf.h"
#include "klee/klee.h"
#include "stdbool.h"
// separate f
extern char *phli_map2[];
extern char *my_map[];
// arraies
extern unsigned char *phli_create_unsigned_char_array(char *name);
extern char *phli_create_char_array(char *name);
extern int *phli_create_int_array(char *name);
extern size_t *phli_create_size_t_array(char *name);
extern unsigned int *phli_create_unsigned_int_array(char *name);
extern unsigned char *phli_create_unsigned_char(char *name);
extern char *phli_create_char(char *name);
extern long *phli_create_long(char *name);
extern int *phli_create_int(char *name);
extern unsigned int *phli_create_unsignedint(char *name);
extern size_t *phli_create_size_t(char *name);
extern uint *phli_create_uint(char *name);
extern bool* phli_create_bool(char *name);
extern void phli_print_bool(bool* val, char* dir);
extern void phli_print_string(char* array, int len, char *dir);
extern void phli_print_int(int *val, char *dir);
extern void phli_print_long2int(long *val, char *dir);
extern void phli_print_long(long *val, char *dir);
extern pval *phli_construct_pval_long(char *name);
extern pval *phli_construct_pval_bool(char *name);
extern pval *phli_construct_pval_double(char *name);
extern pval *phli_construct_pval_string(char *name);
extern void phli_construct_HashTable(HashTable ht);
extern void phli_construct_control_structure_data(control_structure_data re);
extern unsigned char *php_bin2hex(unsigned char*, size_t, size_t*);
/*
int main(int argc, char *argv[]) { 

    char result_dir[] = "/data/phli/results/php_bin2hex/";
    unsigned char* arg_0 = phli_create_unsigned_char_array("arg_0");
	size_t* arg_1 = phli_create_size_t("arg_1");
	size_t* arg_2 = phli_create_size_t_array("arg_2");
    klee_assume(*arg_1 < phli_MAX_ARRAY_SIZE);
	unsigned char* phli_result;
	phli_result = php_bin2hex(arg_0, *arg_1, arg_2);
    phli_print_string(arg_0, (int)arg_1*2 - 1, result_dir);

}
*/

char* _php3_memnstr (char*, char*, int, char*);
/*
int main(int argc, char *argv[]) { 

    char result_dir[] = "/data/phli/results/_php3_memnstr/";
	char* arg_0 = phli_create_char_array("arg_0");
	char* arg_1 = phli_create_char_array("arg_1");
	int* arg_2 = phli_create_int("arg_2");
	char* arg_3 = phli_create_char_array("arg_3");
	char* phli_result;
	phli_result = _php3_memnstr(arg_0, arg_1, *arg_2, arg_3);
    phli_print_string(arg_0, phli_MAX_ARRAY_SIZE, result_dir);
}
*/

// for testing only
/*
int main(int argc, char* argv[]){
    char * arg_0 = phli_create_char_array("arg_0");
    char test[200];
    int len = strlen(arg_0);
    klee_assume(len < 10);
    memcpy(test, arg_0, len);
	char *result_dir = "/data/phli/results/tmp/";
    phli_print_string(test, len, result_dir);
}
*/

//void php3_substr(INTERNAL_FUNCTION_PARAMETERS);
void php3_substr(pval *, pval *, pval *, pval*);
/*
int main(int argc, char *argv[]) { 
	char *result_dir = "/data/phli/results/php3_substr/";
    pval *arg_0 = phli_construct_pval_string("arg_0");
    pval *arg_1 = phli_construct_pval_long("arg_1");
    pval *arg_2 = phli_construct_pval_long("arg_2");
    pval *arg_3 = phli_construct_pval_string("arg_3");
    php3_substr(arg_0, arg_1, arg_2, arg_3);
    //phli_print_string(arg_3->value.str.val, arg_3->value.str.len, result_dir);
    phli_print_int(&arg_3->value.str.len, result_dir);
}
*/



/*
int main(int argc, char *argv[])
{
    char result_dir[] = "/data/phli/results/";
    unsigned char *old = (unsigned char *)malloc(MAX_STRING_VAL_LEN);
    klee_make_symbolic(old, MAX_STRING_VAL_LEN,  "arg1");
    size_t oldlen;
    klee_make_symbolic(&oldlen, sizeof(size_t), "arg2");
    klee_assume(oldlen < 10);
    klee_assume(oldlen > 1);
    size_t newlen;
    klee_make_symbolic(&newlen, sizeof(size_t), "arg3");
    char *rr = NULL;
    rr = php_bin2hex(old, oldlen, &newlen);
    //printf("outside %lx\n", rr);
    //printf("sizeof %d\n", sizeof(rr));
    char filename[50];
    for(int i = 0;i < oldlen * 2;i ++){
        strcpy(filename, result_dir);
        klee_print_expr(my_map[i], rr[i], result_dir);
    }
}
*/



