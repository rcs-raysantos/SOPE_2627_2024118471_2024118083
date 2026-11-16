#include "header.h"

static void sigchld_handler(int sig){
    int saved_errno = errno;

    (void)sig;
    while(waitpid(-1, NULL, WNOHANG) > 0){}
    errno = saved_errno;
}

static void sigint_handler(int sig){
    ssize_t r;

    (void)sig;
    r = write(STDOUT_FILENO, "\n", 1);
    (void)r;
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}

void setup_signals(void){
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if(sigaction(SIGCHLD, &sa, NULL) < 0){
        perror("sigaction SIGCHLD");
    }

    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = sigint_handler;
    sa.sa_flags = SA_RESTART;
    if(sigaction(SIGINT, &sa, NULL) < 0){
        perror("sigaction SIGINT");
    }

    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = SIG_IGN;
    if(sigaction(SIGTSTP, &sa, NULL) < 0){
        perror("sigaction SIGTSTP");
    }
}

void reset_signals_child(void){
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = SIG_DFL;

    if(sigaction(SIGINT, &sa, NULL) < 0){
        perror("sigaction SIGINT");
    }
    if(sigaction(SIGTSTP, &sa, NULL) < 0){
        perror("sigaction SIGTSTP");
    }
    if(sigaction(SIGCHLD, &sa, NULL) < 0){
        perror("sigaction SIGCHLD");
    }
}
