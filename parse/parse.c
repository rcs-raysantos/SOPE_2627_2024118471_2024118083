/*
 *  parse.c
 *  parse
 *
 *  Created by Pedro Sobral on 6/10/26.
 *
 */


#include "header.h"

CMD *command_list = NULL;

/*
 * Imprime os comandos presentes na lista encadeada,
 * detalhando argumentos e redirecionamentos de ficheiros.
 */
void print_command_list(void) {
    CMD *temp;
    int i;
    
    for (temp = command_list; temp != NULL; temp = temp->next) {
        printf("-----NEW COMMAND-----\n");
        if (temp->name) {
            printf("Name = %s\n", temp->name);
        }
        
        for (i = 0; temp->argv[i] != NULL; i++) {
            printf("argv[%d] = %s\n", i, temp->argv[i]);
        }
        
        if (temp->infile != NULL)  printf("Infile = %s\n", temp->infile);
        if (temp->outfile != NULL) printf("Outfile = %s\n", temp->outfile);
        if (temp->errfile != NULL) printf("Errfile = %s\n", temp->errfile);
    }
}

/*
 * Liberta toda a memória alocada dinamicamente para a lista de comandos.
 * Garante a libertação de strings internas e da própria estrutura do nó.
 */
void free_command_list(void) {
    CMD *current = command_list;
    CMD *next_node = NULL;
    int i;
    
    while (current != NULL) {
        next_node = current->next;
        
        // Libertar o nome do comando
        free(current->name);
        
        // Libertar cada argumento alocado
        for (i = 0; i < MAXARGS && current->argv[i] != NULL; i++) {
            free(current->argv[i]);
        }
        
        // Correção dosLeaks: Verificar e libertar cada ficheiro independentemente
        free(current->infile);
        free(current->outfile);
        free(current->errfile);
        
        // Libertar a estrutura do comando atual
        free(current);
        current = next_node;
    }
    
    command_list = NULL;
}

/*
 * Cria e inicializa um novo nó do tipo CMD e adiciona-o ao fim da lista encadeada.
 */
struct command *insert_command(void) {
    int i;
    CMD *new_cmd = (CMD *)malloc(sizeof(CMD));
    
    if (new_cmd == NULL) {
        perror("Erro ao alocar memória com malloc!");
        exit(EXIT_FAILURE);
    }
    
    // Inicialização completa para evitar o acesso a ponteiros inválidos
    new_cmd->name = NULL;
    for (i = 0; i < MAXARGS; i++) {
        new_cmd->argv[i] = NULL;
    }
    new_cmd->infile = NULL;
    new_cmd->outfile = NULL;
    new_cmd->errfile = NULL;
    new_cmd->next = NULL;
    
    // Inserção no fim da lista encadeada
    if (command_list == NULL) {
        command_list = new_cmd;
    } else {
        CMD *temp = command_list;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = new_cmd;
    }
    
    return new_cmd;
}

/*
 * Analisa uma linha de comandos (string) e preenche a lista encadeada de comandos.
 */
void parse_line(char *line) {
    char *a;
    int count = 0;
    char *infile, *outfile, *errfile;
    CMD *command;
    
    // Se já existirem comandos guardados na lista anterior, liberta-os
    if (command_list != NULL) {
        free_command_list();
    }
    
    if (line == NULL || strlen(line) == 0) {
        return;
    }

    command = insert_command();
    
    for (a = strtok(line, " \t\r\n"); a != NULL; a = strtok(NULL, " \t\r\n")) {
        // Redirecionamento de pipeline '|'
        if (a[0] == '|') {
            // Garante que o argumento do nó anterior termina a NULL
            if (count < MAXARGS) {
                command->argv[count] = NULL;
            }
            command = insert_command();
            count = 0;
        }
        // Redirecionamento de entrada '<'
        else if (a[0] == '<') {
            infile = a[1] ? a + 1 : strtok(NULL, " \t\r\n");
            if (infile) {
                free(command->infile); // Prevenção contra múltiplos reencaminhamentos
                command->infile = strdup(infile);
            }
        }
        // Redirecionamento de saída '>'
        else if (a[0] == '>') {
            outfile = a[1] ? a + 1 : strtok(NULL, " \t\r\n");
            if (outfile) {
                free(command->outfile);
                command->outfile = strdup(outfile);
            }
        }
        // Redirecionamento de erro '2>'
        else if (a[0] == '2' && a[1] == '>') {
            errfile = a[2] ? a + 2 : strtok(NULL, " \t\r\n");
            if (errfile) {
                free(command->errfile);
                command->errfile = strdup(errfile);
            }
        }
        // Comandos e Argumentos
        else {
            if (count < MAXARGS - 1) { // Previne overflow do buffer argv
                if (count == 0) {
                    command->name = strdup(a);
                }
                command->argv[count] = strdup(a);
                count++;
            }
        }
    }
    
    if (count < MAXARGS) {
        command->argv[count] = NULL;
    }
}
