/*
 *  main.c
 *  parse
 */

#include "header.h"

#define ANSI_GREEN "\x1b[32m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_BLUE "\x1b[34m"
#define ANSI_BOLD "\x1b[1m"
#define ANSI_RESET "\x1b[0m"

void build_prompt(char *str, size_t size){
    char hostname[256], cwd[1024], *username = getenv("USER"); // pega o nome do user

    if(gethostname(hostname, sizeof(hostname)) != 0){ // retorna 0 se funcionar
        strncpy(hostname, "localhost", sizeof(hostname)); // nome default, ou seja, caso dê errado
    }

    if(getcwd(cwd, sizeof(cwd)) == NULL){
        strncpy(cwd, "??", sizeof(cwd));
    }

    if(username == NULL){
        username = "user";
    }

    snprintf(str, size,
    "%s[msh]%s %s%s@%s%s:%s%s%s$ ", 
    ANSI_BOLD ANSI_YELLOW, ANSI_RESET, ANSI_BOLD ANSI_GREEN,
    username, hostname, ANSI_RESET,
    ANSI_BOLD ANSI_BLUE, cwd, ANSI_RESET);
}

int main(int argc, const char *argv[]) {
    char *line, prompt[2048];
    
    while (1) {
        build_prompt(prompt, sizeof(prompt)); // cria o promptzinho
        line = readline(prompt); // apresenta pro user
        
        // Trata o sinal EOF (Ctrl+D)
        if (line == NULL) {
            printf("\nExiting...\n");
            break;
        }
        
        // Verifica se a linha digitada é "exit"
        if (strcmp(line, "exit") == 0) {
            free(line); // Libertação da memória de readline antes de sair
            break;
        }
        
        // Adiciona à história caso a linha não esteja vazia
        if (*line) {
            add_history(line);
        }
        
        parse_line(line);
        print_command_list();
        free_command_list();
        
        free(line); // Liberta a memória alocada por readline
    }
    
    return 0;
}
