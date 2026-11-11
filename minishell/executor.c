#include "header.h"

void builtin_help(void){
    printf("Comandos Internos Disponiveis:\n");
    printf("cd <diretoria> -> Altera a diretoria de trabalho atual.\n");
    printf("pwd -> Mostra o caminho absoluto do diretorio atual.\n");
    printf("exit -> Termina graciosamente a minishell.\n");
    printf("history -> Apresenta a lista dos ultimos comandos introduzidos.\n");
    printf("echo -> Apresenta no ecra o texto indicado como argumento.\n");
    printf("help -> Apresenta este menu de ajuda.\n"); // --> retirar?? <--
}

void builtin_cd(CMD *cmd){
    char *pasta = cmd->argv[1];

    if(pasta == NULL){
        pasta = getenv("HOME");
        
        if(pasta == NULL){
            perror("cd: variavel HOME nao definida.");
            return;
        }
    }

    if(chdir(pasta) != 0){ // faz a mudança de diretorio para «pasta»
        perror("chdir: não conseguiu fazer a mudança.");
    }
}

void builtin_pwd(void){
    char cwd[1024];

    if(getcwd(cwd, sizeof(cwd)) != NULL){ // pega o caminho absoluto do diretorio atual
        printf("%s\n", cwd);    
    }
    perror("cwd: nao conseguiu pegar o caminho absoluto.");
}

void builtin_exit(void){ 
    exit(0); // o processo termina normalmente
}

void builtin_history(void){
    HIST_ENTRY **historico = history_list();

    if(historico != NULL){
        for(int i = 0; historico != NULL; i++){
            printf("%d  %s\n", i + 1, historico[i]->line);
        }
    }
}

void builtin_echo(CMD *cmd){
    for(int i = 1; cmd->argv[i] != NULL; i++){
        printf("%s", cmd->argv[i]);

        if(cmd->argv[i + 1] != NULL){
            printf(" ");
        }
    }
    
    printf("\n");
}

int execute_int(CMD *cmd){ // 1 = é interno | 0 = é externo
    if(strcmp(cmd->name, "cd") == 0){
        builtin_cd(cmd);
        return 1;
    }

    if(strcmp(cmd->name, "pwd") == 0){
        builtin_pwd();
        return 1;
    }
    
    if(strcmp(cmd->name, "echo") == 0){
        builtin_echo(cmd);
        return 1;
    }

    if(strcmp(cmd->name, "history") == 0){
        builtin_history();
        return 1;
    }

    if(strcmp(cmd->name, "help") == 0){
        builtin_help();
        return 1;
    }

    if(strcmp(cmd->name, "exit") == 0){
        builtin_exit();
        return 1;
    }

    return 0; 
}

void execute_ext(CMD *cmd){
    int pid = fork();

    if (pid < 0){
        perror("fork");
        return;
    }

    if (pid == 0){ //filho
        execvp(cmd->name, cmd->argv);
        perror("execvp: comando nao encontrado.");
        exit(-1);
    } else { //pai
        int status;
        if (waitpid(pid, &status, 0) < 0){
            perror("waitpid");
        }
    }
}

void execute_commands(void){
    CMD *aux = command_list;

    while(aux != NULL){
        if(aux->name == NULL){
            aux = aux->next;
            continue;
        }

        if(execute_int(aux) == 0){ // executa no pai
            execute_ext(aux); // executa no filho
        } 

        aux = aux->next;
    }
}