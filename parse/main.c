/*
 *  main.c
 *  parse
 */

#include "header.h"

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_BOLD "\x1b[1m"
#define ANSI_COLOR_RESET "\x1b[0m"

void build_prompt(){
    char hostname[256], cwd[1024], *username = getenv("USER");

    if(){

    }

    if(){

    }

    if(){

    }

    snprintf();
}

int main(int argc, const char *argv[]) {
    char *line;
    
    while (1) {
        line = readline("msh$ ");
        
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
