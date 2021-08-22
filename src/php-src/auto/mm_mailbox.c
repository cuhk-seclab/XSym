#include "stdio.h"
#include "phli.h"
void mm_mailbox(char*);
int main(int argc, char* argv[]) {
char *result_dir="/data/phli/results/mm_mailbox/";
char* arg_0=phli_create_char_array("arg_0");
mm_mailbox(arg_0);
}