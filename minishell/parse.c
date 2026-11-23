/**
 * @file parse.c
 * @brief Implementação do analisador léxico/sintático (parser) de linhas de comandos.
 */

#include "header.h"

/** @brief Ponteiro global para o início da lista ligada de comandos atual. */
CMD *command_list = NULL;

/**
 * @brief Verifica e remove o sufixo `&` de uma string, ativando a flag de background.
 *
 * Se a string `s` não for `NULL`, tiver comprimento superior a 1 e terminar
 * com o carácter `'&'`, substitui-o por `'\0'` (modificando `s` in-place) e
 * define `command->background = 1`.
 *
 * @param s String a verificar e eventualmente truncar (tipicamente um nome de ficheiro).
 * @param command Ponteiro para a estrutura `CMD` cuja flag `background` será atualizada.
 */
static void strip_bg(char *s, CMD *command) {
    size_t len;

    if (s == NULL) {
        return;
    }

    len = strlen(s);
    if (len > 1 && s[len - 1] == '&') {
        s[len - 1] = '\0';
        command->background = 1;
    }
}

/**
 * @brief Imprime os comandos presentes na lista ligada.
 *
 * Percorre `command_list` e apresenta no `stdout` o nome, os argumentos
 * (`argv`) e os ficheiros de redirecionamento (`infile`, `outfile` e `errfile`)
 * de cada nó `CMD`, sendo útil para depuração.
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

/**
 * @brief Liberta toda a memória alocada dinamicamente para a lista de comandos.
 *
 * Percorre `command_list`, libertando o nome do comando, todas as strings
 * alocadas em `argv`, os caminhos de redirecionamento (`infile`, `outfile`,
 * `errfile`) e cada estrutura `CMD`, repondo `command_list` a `NULL`.
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

/**
 * @brief Cria e inicializa um novo nó `CMD`, inserindo-o no fim da lista ligada.
 *
 * Aloca memória para uma nova estrutura `CMD`, inicializa todos os seus ponteiros
 * a `NULL` e flags a `0`, e anexa-a ao final de `command_list`. Em caso de
 * falha de alocação, imprime erro com `perror` e termina o programa com
 * `EXIT_FAILURE`.
 *
 * @return struct command* Ponteiro para a nova estrutura `CMD` inserida na lista.
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
    new_cmd->append = 0;
    new_cmd->background = 0;
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

/**
 * @brief Analisa uma linha de comandos e preenche a lista ligada `command_list`.
 *
 * Liberta eventuais comandos anteriores em `command_list`. Se `line` for válida
 * e não vazia, divide-a em tokens usando `strtok` (modificando `line` in-place)
 * e interpreta:
 * - Pipes (`|`), criando novos nós `CMD` na lista;
 * - Redirecionamentos de entrada (`<`), saída (`>`, `>>`) e erro (`2>`);
 * - Execução em segundo plano (`&`, isolado ou colado ao último token);
 * - Nome do comando e até `MAXARGS - 1` argumentos terminados em `NULL`.
 *
 * @param line String mutável contendo a linha de comandos a analisar.
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
                strip_bg(infile, command);
                free(command->infile); // Prevenção contra múltiplos reencaminhamentos
                command->infile = strdup(infile);
            }
        }
        else if (a[0] == '>' && a[1] == '>') {
            outfile = a[2] ? a + 2 : strtok(NULL, " \t\r\n");
            if (outfile) {
                strip_bg(outfile, command);
                free(command->outfile);
                command->outfile = strdup(outfile);
                command->append = 1;
            }
        }
        // Redirecionamento de saída '>'
        else if (a[0] == '>') {
            outfile = a[1] ? a + 1 : strtok(NULL, " \t\r\n");
            if (outfile) {
                strip_bg(outfile, command);
                free(command->outfile);
                command->outfile = strdup(outfile);
                command->append = 0;
            }
        }
        // Redirecionamento de erro '2>'
        else if (a[0] == '2' && a[1] == '>') {
            errfile = a[2] ? a + 2 : strtok(NULL, " \t\r\n");
            if (errfile) {
                strip_bg(errfile, command);
                free(command->errfile);
                command->errfile = strdup(errfile);
            }
        }
        
        // Operador de background '&'
        else if (strcmp(a, "&") == 0) {
            command->background = 1;
        }
        // Comandos e Argumentos
        else {
            size_t len = strlen(a);

            if (len > 1 && a[len - 1] == '&') { // caso "sleep 10&"
                a[len - 1] = '\0';
                command->background = 1;
            }

            if (count < MAXARGS - 1) {
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