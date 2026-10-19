/*
 *  main.c
 *  parse
 */

#include "header.h"

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
