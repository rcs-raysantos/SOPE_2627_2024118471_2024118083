#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

/**
 * @file header.h
 * @brief Definições globais, estruturas de dados e protótipos da minishell.
 *
 * Created by Pedro Sobral on 06/10/26.
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

/** @brief Código ANSI para repor a formatação e cor padrão do terminal. */
#define ANSI_RESET "\001\x1b[0m\002"

/** @brief Código ANSI para ativar texto em negrito. */
#define ANSI_BOLD "\001\x1b[1m\002"

/** @brief Código ANSI para cor magenta em negrito (roxo). */
#define ANSI_MAGENTA "\001\x1b[1;35m\002" // Roxo 

/** @brief Código ANSI para cor ciano em negrito. */
#define ANSI_CYAN "\001\x1b[1;36m\002" // Ciano

/** @brief Código ANSI para cor amarela em negrito. */
#define ANSI_YELLOW "\001\x1b[1;33m\002" // Amarelo

/** @brief Número máximo de argumentos suportados por comando (incluindo o nome e o `NULL` final). */
#define MAXARGS 10

/**
 * @struct command
 * @brief Representa um comando individual e os respetivos atributos de execução.
 *
 * Estrutura usada como nó de uma lista ligada para representar comandos simples
 * ou pipelines de comandos, incluindo argumentos, redirecionamentos e execução
 * em segundo plano.
 */
typedef struct command {
    char *name;             /**< Nome do comando a executar. */
    char *argv[MAXARGS];    /**< Vetor de argumentos do comando terminado em `NULL`. */
    char *infile;           /**< Caminho do ficheiro de redirecionamento de entrada (`<`), ou `NULL`. */
    char *outfile;          /**< Caminho do ficheiro de redirecionamento de saída (`>` / `>>`), ou `NULL`. */
    char *errfile;          /**< Caminho do ficheiro de redirecionamento de erro (`2>`), ou `NULL`. */
    int append;             /**< Flag que indica anexação na saída (`1` para `>>`, `0` para `>`). */
    int background;         /**< Flag que indica execução em segundo plano (`1` para `&`, `0` caso contrário). */
    struct command *next;   /**< Ponteiro para o próximo comando na lista ligada (pipe), ou `NULL`. */
} CMD;

/**
 * @brief Ponteiro global para o início da lista ligada de comandos atual.
 */
extern CMD *command_list;

/**
 * @brief Insere uma nova estrutura `CMD` na lista de comandos.
 *
 * Aloca e inicializa um novo nó `CMD`, adicionando-o à lista global `command_list`.
 *
 * @return CMD* Ponteiro para a estrutura `CMD` recém-criada (ou `NULL` em caso de falha).
 */
CMD *insert_command();

/**
 * @brief Liberta toda a memória alocada pela lista de comandos.
 *
 * Percorre `command_list`, libertando os campos alocados dinamicamente e os
 * respetivos nós da lista, repondo o ponteiro global para `NULL`.
 */
void free_command_list(void);

/**
 * @brief Imprime o conteúdo da lista de comandos atual.
 *
 * Apresenta no ecrã os detalhes de cada comando presente em `command_list`
 * (nome, argumentos, redirecionamentos e flags), útil para depuração.
 */
void print_command_list(void);

/**
 * @brief Analisa uma linha de comando introduzida pelo utilizador.
 *
 * Efetua o parsing da string de entrada, separando comandos ligados por pipes,
 * argumentos, redirecionamentos de E/S e execução em background, preenchendo
 * a lista global `command_list`.
 *
 * @param line String com a linha de comandos lida pela shell.
 */
void parse_line(char *line);

/**
 * @brief Constrói a string do prompt apresentada ao utilizador.
 *
 * Formata o texto do prompt (com as cores ANSI e informação do ambiente/diretoria)
 * para o buffer indicado.
 *
 * @param str Buffer de destino onde o prompt será escrito.
 * @param size Tamanho máximo disponível no buffer `str`.
 */
void build_prompt(char *str, size_t size);

/**
 * @brief Executa os comandos presentes na lista global `command_list`.
 *
 * Gere a execução de comandos internos, comandos externos, redirecionamentos
 * e pipelines de comandos.
 *
 * @return int Retorna `-1` se o comando `exit` for invocado para encerrar a
 *             minishell, ou `0` nos restantes casos.
 */
int execute_commands();

/**
 * @brief Configura o tratamento de sinais no processo principal da minishell.
 */
void setup_signals(void);

/**
 * @brief Repõe os manipuladores de sinais para o comportamento padrão nos processos filhos.
 */
void reset_signals_child(void);