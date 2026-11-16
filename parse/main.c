/*
 *  main.c
 *  parse
 */

#include "header.h"

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
        "%s[msh]%s %s%s@%s%s:%s%s%s ➜ ",
        ANSI_MAGENTA, ANSI_RESET,
        ANSI_CYAN, username, hostname, ANSI_RESET,
        ANSI_YELLOW, cwd, ANSI_RESET);
}

int main(void) {
    char *line, prompt[2048];

    setup_signals();
    
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

        int status = execute_commands();
        
        free_command_list();
        //print_command_list();

        free(line); // Liberta a memória alocada por readline
        
        if (status == -1) {
            break; // sai do loop para encerrar o programa
        }
    }
    
    clear_history();
    return 0;
}
