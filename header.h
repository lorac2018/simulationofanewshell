#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
/*
 *  header.h
 *  parse
 *
 *  Created by Pedro Sobral on 09/01/23.
 *  Copyright 2009 UFP. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <signal.h>
#include <memory.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <pwd.h>
#include <grp.h>
#define MAXARGS 10
#define VERMELHO  "\x1B[31m\e[1m"
#define VERDE  "\x1B[32m\e[1m"
#define AZUL  "\x1B[34m\e[1m"
#define CYAN  "\x1B[36m\e[1m"
#define BRANCO  "\e[97m\e[0m"

typedef struct command {
    char *name;
    char *argv[MAXARGS];
    char *infile;
    char *outfile;
    char *errfile;
    struct command *next;
} CMD;

CMD *insert_command();
void free_command_list();
void print_command_list();
void exec_comandos(CMD* root);
int n_comandos(CMD *root);
void atualizar_caminho();
CMD * parse_line(char *);
int parse_path();
void myexec(CMD* root);
void *preencher_biblioteca(void *pos);
char **character_name_completion(const char *, int, int);
char *character_name_generator(const char *, int);
void *listdir(void *name);
void *produtor(void *name);
void *consumidor(void *name);