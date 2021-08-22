#include "phli.h"
#include "klee/klee.h"
char *phli_map2[33] = { "0","1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19","20","21","22","23","24","25","26","27","28","29","30","31","32"};
char *my_map[] = {
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
    "11", "12", "13", "14", "15", "16", "17", "18", "19", "20",
    "21", "22", "23", "24", "25", "26", "27", "28", "29", "30",
    "31", "32", "33", "34", "35", "36", "37", "38", "39", "40",
    "41", "42", "43", "44", "45", "46", "47", "48", "49", "50",
    "51", "52", "53", "54", "55", "56", "57", "58", "59", "60",
    "61", "62", "63", "64", "65", "66", "67", "68", "69", "70",
    "71", "72", "73", "74", "75", "76", "77", "78", "79", "80",
    "81", "82", "83", "84", "85", "86", "87", "88", "89", "90",
    "91", "92", "93", "94", "95", "96", "97", "98", "99", "100", "101"};


// arraies
unsigned char *phli_create_unsigned_char_array(char *name){
    unsigned char *old = (unsigned char *) malloc(phli_MAX_ARRAY_SIZE * sizeof(unsigned char));
    klee_make_symbolic(old, phli_MAX_ARRAY_SIZE * sizeof(unsigned char), name);
    *(old + phli_MAX_ARRAY_SIZE - 1) = '\0'; // @todo test
    return old;
}

char *phli_create_char_array(char *name){
    char *old = (char *)malloc(phli_MAX_ARRAY_SIZE * sizeof(char));
    klee_make_symbolic(old, phli_MAX_ARRAY_SIZE * sizeof(char), name);
    *(old + phli_MAX_ARRAY_SIZE - 1) = '\0'; // @todo test
    return old;
}

int *phli_create_int_array(char *name){
    int *old = (int *)malloc(phli_MAX_ARRAY_SIZE * sizeof(int));
    klee_make_symbolic(old, phli_MAX_ARRAY_SIZE * sizeof(int), name);
    return old;
}

size_t *phli_create_size_t_array(char *name){
    size_t *old = (size_t *)malloc(phli_MAX_ARRAY_SIZE * sizeof(size_t));
    klee_make_symbolic(old, phli_MAX_ARRAY_SIZE * sizeof(size_t), name);
    return old;
}
unsigned int *phli_create_unsigned_int_array(char *name){
    unsigned int *old = (unsigned int *)malloc(phli_MAX_ARRAY_SIZE * sizeof(unsigned int));
    klee_make_symbolic(old, phli_MAX_ARRAY_SIZE * sizeof(unsigned int), name);
    return old;
}





// single variable
unsigned char *phli_create_unsigned_char(char *name){
    unsigned char *old = (unsigned char *)malloc(sizeof(unsigned char));
    klee_make_symbolic(old, sizeof(unsigned char), name);
    return old;
}

bool *phli_create_bool(char* name){
    bool *old = (bool*) malloc(sizeof(bool));
    klee_make_symbolic(old, sizeof(bool), name);
    return old;
} 
char *phli_create_char(char *name){
    char *old = (char *)malloc(sizeof(char));
    klee_make_symbolic(old, sizeof(char), name);
    return old;
}

long *phli_create_long(char *name){
    long *old = (long *)malloc(sizeof(long));
    klee_make_symbolic(old, sizeof(long), name);
    return old;
}

int *phli_create_int(char *name){
    int *old = (int *)malloc(sizeof(int));
    klee_make_symbolic(old, sizeof(int), name);
    return old;
}

unsigned int *phli_create_unsignedint(char *name){
    unsigned int *old = (unsigned int *)malloc(sizeof(unsigned int));
    klee_make_symbolic(old, sizeof(unsigned int), name);
    return old;
}

size_t *phli_create_size_t(char *name){
    size_t *old = (size_t *)malloc(sizeof(size_t));
    klee_make_symbolic(old, sizeof(size_t), name);
    return old;
}


uint *phli_create_uint(char *name){
    uint *old = (uint *)malloc(sizeof(uint));
    klee_make_symbolic(old, sizeof(uint), name);
    return old;
}

void phli_print_string(char* array, int len, char *dir){
    if (len == 0){
        klee_print_expr(my_map[0], '\0', dir);
    }
    for(int i = 0; i < len; i ++){
        klee_print_expr(my_map[i], array[i], dir);
    }
}

void phli_print_int(int *val, char *dir){
    klee_print_expr("int", *val, dir);
}

void phli_print_long2int(long *val, char *dir){
    klee_print_expr("long_int", *(int*)val, dir);
    klee_print_expr("long_int_1", *((int*)val + 1), dir);
}

void phli_print_long(long *val, char *dir){
    klee_print_expr("long", *val, dir);
}

void phli_print_bool(bool *val, char *dir){
    klee_print_expr("bool", *val, dir);
}

pval *phli_construct_pval_bool(char *name){
    pval *re = (pval*)malloc(phli_PVAL_SIZE);
    re->type = IS_BOOL;
    char *value_name = (char*)malloc(40);
    strcpy(value_name, name);
    bool *tmp=phli_create_bool(strcat(value_name, "bool_blval"));
    re->value.blval = *tmp;
    free(value_name);
    return re;
}

pval *phli_construct_pval_long(char *name){
    pval *re = (pval *)malloc(phli_PVAL_SIZE);
    //klee_make_symbolic(re, phli_PVAL_SIZE, name);// 24 bytes
    re->type = IS_LONG;
    char *value_name = (char *)malloc(40);
    strcpy(value_name, name);
    long *tmp = phli_create_long(strcat(value_name, "long_lval"));
    re->value.lval = *tmp;
    free(value_name);
    return re;
}

pval *phli_construct_pval_double(char *name){

    pval *re = (pval *)malloc(phli_PVAL_SIZE);
    klee_make_symbolic(re, phli_PVAL_SIZE, name);// 24 bytes
    re->type = IS_DOUBLE;
    char *value_name = (char *)malloc(40);
    strcpy(value_name, name);
    //re->value.lval = phli_create_double(strcat(value_name, "long_lval"))
    free(value_name);
    return re;
}
pval *phli_construct_pval_string(char *name){

    pval *re = (pval *)malloc(phli_PVAL_SIZE);
    //klee_make_symbolic(re, phli_PVAL_SIZE, name);// 24 bytes
    re->type = IS_STRING;// @todo
    char *value_name = (char *)malloc(40);
    strcpy(value_name, name);
    re->value.str.val =  phli_create_char_array(strcat(value_name, "str_val"));
    free(value_name);
    re->value.str.len =  strlen(re->value.str.val);
    return re;
}


void phli_construct_HashTable(HashTable ht){
}

void phli_construct_control_structure_data(control_structure_data re){
}

