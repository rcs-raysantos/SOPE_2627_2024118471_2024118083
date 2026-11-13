#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>

/* Ativa as funcoes POSIX (strdup, gethostname, chdir, etc.) no modo C11 */
/* Códigos de cor ANSI compatíveis com Readline (\001 e \002 evitam bugs no terminal) */
#define ANSI_RESET "\001\x1b[0m\002"
#define ANSI_BOLD "\001\x1b[1m\002"
#define ANSI_MAGENTA "\001\x1b[1;35m\002" // Roxo 
#define ANSI_CYAN "\001\x1b[1;36m\002" // Ciano
#define ANSI_YELLOW "\001\x1b[1;33m\002" // Amarelo

#define MAXARGS 10

typedef struct command {
    char *name;
    char *argv[MAXARGS];
    char *infile;
    char *outfile;
    char *errfile;
    int append;
    int background;
    struct command *next;
} CMD;

extern CMD *command_list;

CMD *insert_command();
void free_command_list(void);
void print_command_list(void);
void parse_line(char *);

void build_prompt(char *str, size_t size);
int execute_commands();