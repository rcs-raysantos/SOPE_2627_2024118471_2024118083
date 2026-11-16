/**
 * @file main.c
 * @brief Ponto de entrada e ciclo principal (REPL) da minishell.
 */

#include "header.h"

/**
 * @brief Constrói a string formatada do prompt da minishell.
 *
 * Obtém o nome de utilizador (`USER`), o nome do anfitrião (`gethostname`) e
 * a diretoria de trabalho atual (`getcwd`), aplicando valores por omissão
 * (`"user"`, `"localhost"` e `"??"`, respetivamente) caso alguma obtenção falhe.
 * Formata o resultado com cores ANSI no formato `[msh] user@host:cwd ➜ ` e
 * guarda-o no buffer `str` respeitando o limite `size`.
 *
 * @param str Buffer de destino onde a string do prompt será armazenada.
 * @param size Tamanho máximo do buffer `str` em bytes.
 */
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

/**
 * @brief Função principal da minishell.
 *
 * Inicializa o tratamento de sinais com `setup_signals()` e entra no ciclo
 * interativo (Read-Eval-Print Loop):
 * - Constrói o prompt e lê comandos com `readline()`.
 * - Trata fim de ficheiro (`Ctrl+D` / `EOF`) e a instrução direta `"exit"`.
 * - Adiciona linhas não vazias ao histórico da biblioteca Readline.
 * - Efetua o parsing (`parse_line`) e executa os comandos (`execute_commands`).
 * - Liberta os recursos alocados (`free_command_list` e `free(line)`) a cada iteração.
 * - Limpa o histórico (`clear_history`) antes de encerrar.
 *
 * @return int Retorna `0` após o encerramento normal da minishell.
 */
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