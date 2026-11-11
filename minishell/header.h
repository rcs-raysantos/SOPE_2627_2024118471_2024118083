/*
 *  header.h
 *  parse
 *
 *  Created by Pedro Sobral on 06/10/26.
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

#define ANSI_GREEN "\x1b[32m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_BLUE "\x1b[34m"
#define ANSI_BOLD "\x1b[1m"
#define ANSI_RESET "\x1b[0m"

#define MAXARGS 10

typedef struct command {
    char *name;
    char *argv[MAXARGS];
    char *infile;
    char *outfile;
    char *errfile;
    struct command *next;
} CMD;

extern CMD *command_list;

void build_prompt(char *str, size_t size);


CMD *insert_command();
void free_command_list(void);
void print_command_list(void);
void parse_line(char *);
void build_prompt(char *str, size_t size);

void builtin_help();
void builtin_cd(CMD *cmd);
void builtin_pwd();
void builtin_exit();
void builtin_history();
void builtin_echo(CMD *cmd);
int execute_int(CMD *cmd); // interno
void execute_ext(CMD *cmd); // externo
void execute_commands(); // executa ambos