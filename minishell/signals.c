/**
 * @file signals.c
 * @brief Tratamento de sinais da minishell (SIGCHLD, SIGINT e SIGTSTP).
 */

#include "header.h"

/**
 * @brief Rotina de atendimento (handler) para o sinal SIGCHLD.
 *
 * Recolhe de forma assíncrona o estado dos processos filhos (em background)
 * que já terminaram, utilizando waitpid() com a flag WNOHANG para não bloquear
 * a minishell e evitar a acumulação de processos zombie.
 *
 * @param sig Identificador do sinal recebido.
 */
static void sigchld_handler(int sig){
    if(sig == SIGCHLD){
        while(waitpid(-1, NULL, WNOHANG) > 0); // WNOHANG = espera sem bloquear o processo
    }
}

/**
 * @brief Rotina de atendimento (handler) para o sinal SIGINT (Ctrl+C).
 *
 * Impede que a minishell feche ao carregar em Ctrl+C, limpando a linha
 * atual no terminal e apresentando um novo prompt através do Readline.
 *
 * @param sig Identificador do sinal recebido.
 */
static void sigint_handler(int sig){
    if(sig == SIGINT){
        printf("\n");
        rl_on_new_line();
        rl_replace_line("", 0);
        rl_redisplay();
    }
}

/**
 * @brief Configura o tratamento de sinais no processo pai (minishell).
 *
 * - SIGCHLD: configurado com sigaction() e SA_RESTART para recolher zombies.
 * - SIGINT (Ctrl+C): associado à rotina trata_sigint.
 * - SIGTSTP (Ctrl+Z): ignorado (SIG_IGN) para não suspender a shell.
 */
void setup_signals(void){
    struct sigaction sa;

    sa.sa_handler = trata_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;

    if(sigaction(SIGCHLD, &sa, NULL) < 0){
        perror("sigaction: SIGCHLD");
    }

    if(signal(SIGINT, trata_sigint) == SIG_ERR){ // ctrl+c
        perror("signal: SIGINT");
    }

    if(signal(SIGTSTP, SIG_IGN) == SIG_ERR){ // ctrl+z
        perror("signal: SIGTSTP");
    }
}

/**
 * @brief Repõe o comportamento por omissão (SIG_DFL) dos sinais no processo filho.
 *
 * Chamada após o fork() nos processos filhos para que os comandos executados
 * possam reagir normalmente ao Ctrl+C e Ctrl+Z.
 */
void reset_signals_child(void){
    if(signal(SIGINT, SIG_DFL) == SIG_ERR){
        perror("signal: SIGINT");
    }

    if(signal(SIGTSTP, SIG_DFL) == SIG_ERR){
        perror("signal: SIGTSTP");
    }

    if(signal(SIGCHLD, SIG_DFL) == SIG_ERR){
        perror("signal: SIGCHLD");
    }
}
