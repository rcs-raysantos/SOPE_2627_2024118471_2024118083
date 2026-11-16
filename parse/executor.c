#include "header.h"

void builtin_help(void){
    printf("-------- COMANDOS INTERNOS DISPONIVEIS: --------\n");
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
    } else {
        perror("cwd: nao conseguiu pegar o caminho absoluto.");
    }
}

void builtin_history(void){
    HIST_ENTRY **historico = history_list();

    if(historico != NULL){
        for(int i = 0; historico[i] != NULL; i++){
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
        return -1; //sinal para terminar!
    }

    return 0; 
}

static int apply_redirections(CMD *cmd){
    int fd;

    if(cmd->infile != NULL){
        fd = open(cmd->infile, O_RDONLY);
        if(fd < 0){ perror(cmd->infile); return -1; }
        if(dup2(fd, STDIN_FILENO) < 0){ perror("dup2 -> STDIN"); close(fd); return -1; }
        close(fd);
    }

    if(cmd->outfile != NULL){
        int flags = O_WRONLY | O_CREAT | (cmd->append ? O_APPEND : O_TRUNC);
        fd = open(cmd->outfile, flags, 0644);
        if(fd < 0){ perror(cmd->outfile); return -1; }
        if(dup2(fd, STDOUT_FILENO) < 0){ perror("dup2 -> STDOUT"); close(fd); return -1; }
        close(fd);
    }

    if(cmd->errfile != NULL){
        fd = open(cmd->errfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if(fd < 0){ perror(cmd->errfile); return -1; }
        if(dup2(fd, STDERR_FILENO) < 0){ perror("dup2 -> STDERR"); close(fd); return -1; }
        close(fd);
    }

    return 0;
}

void execute_ext(CMD *cmd){
    pid_t pid = fork();

    if (pid < 0){
        perror("fork");
        return;
    }

    if (pid == 0){ //filho
        reset_signals_child();
        if(cmd->background){
            setpgid(0, 0);
        }
        if(apply_redirections(cmd) < 0){ _exit(EXIT_FAILURE); }
        execvp(cmd->name, cmd->argv);
        perror("execvp: comando nao encontrado.");
        _exit(EXIT_FAILURE);
    }

    if (cmd->background){
        printf("[Processo em background iniciado com PID %d]\n", pid);
        return; // NAO faz waitpid -> o prompt volta logo
    }
    
    int status;
    if (waitpid(pid, &status, 0) < 0 && errno != ECHILD) {
        perror("waitpid");
    }
    
}

int execute_pipe(void){
    CMD *cmd;
    int fds[2];
    int prev_fd = -1; // começa com -1 pq dps vai receber um fds

    int count_cmd = 0;

    for(CMD *c = command_list; c != NULL; c = c->next){
        if(c->name != NULL){
            count_cmd++;
        }
    }

    if(count_cmd == 0){
        return 0; //nada a executar
    }

    pid_t *pids = malloc(count_cmd * sizeof(pid_t)); 
    if(pids == NULL){
        perror("malloc");
        return 0;
    }

    int cmd_id = 0;
    cmd = command_list;
    int background = 0;
    for(CMD *c = command_list; c != NULL; c = c->next){
        if(c->name != NULL){
            background = c->background; // fica com o do ultimo comando
        }
    }
    
    while(cmd != NULL){
        if(cmd->name == NULL){
            cmd = cmd->next;
            continue;
        }

        if(cmd->next != NULL){
            if(pipe(fds) < 0){
                perror("pipe");
                free(pids);
                return 0;
            }
        }

        pid_t pid = fork();

        if(pid < 0){
            perror("fork");

             if(cmd->next != NULL){ // se havia proximos comandos, fechamos os fds
                close(fds[0]);
                close(fds[1]);
            }

            if (prev_fd != -1) { // fechamos file descriptor passado
                close(prev_fd);
            }

            free(pids); // libertamos espaço da memoria
            return 0;
        }

        if(pid == 0){ // filho
            reset_signals_child();
            if(background){
                setpgid(0, 0);
            }
            if(prev_fd != -1){ // se houver pipe anterior vamos duplicar o pipe anterior para leitura
                if(dup2(prev_fd, STDIN_FILENO) < 0){
                    perror("dup2 -> STDIN");
                    _exit(EXIT_FAILURE);
                }

                close(prev_fd); // fecha o duplicado
            }

            if(cmd->next != NULL){
                close(fds[0]); // fecha o ponto de leitura

                if(dup2(fds[1], STDOUT_FILENO) < 0){ 
                    perror("dup2 -> STDOUT");
                    _exit(EXIT_FAILURE);
                }

                close(fds[1]); // fecha o duplicado
            }

            if(apply_redirections(cmd) < 0){ _exit(EXIT_FAILURE); }

            int val = execute_int(cmd);

            if(val != 0){
                fflush(stdout);
                _exit(0);
            }

            execvp(cmd->name, cmd->argv);
            perror("execvp");
            _exit(EXIT_FAILURE);
        }

        pids[cmd_id++] = pid;

        if(prev_fd != -1){
            close(prev_fd);
        }

        if(cmd->next != NULL){
            close(fds[1]); // fecha a escrita
            prev_fd = fds[0]; // passa a leitura pro proximo fi
        } else {
            prev_fd = -1; // encerra o "ciclo"
        }

        cmd = cmd->next;
    }

    if (background) {
        printf("[Processo em background iniciado com PID %d]\n",
               pids[cmd_id - 1]);
    } else {
        for (int i = 0; i < cmd_id; i++) {
            int status;
            if (waitpid(pids[i], &status, 0) < 0 && errno != ECHILD) {
                perror("waitpid");
            }
        }
    }

    free(pids);

    return 0;
}

static int is_builtin(const char *name){
    return strcmp(name, "cd") == 0 || strcmp(name, "pwd") == 0 ||
           strcmp(name, "echo") == 0 || strcmp(name, "history") == 0 ||
           strcmp(name, "help") == 0 || strcmp(name, "exit") == 0;
}

static int execute_builtin_redir(CMD *cmd){
    int saved_in = dup(STDIN_FILENO);
    int saved_out = dup(STDOUT_FILENO);
    int saved_err = dup(STDERR_FILENO);
    int val = 0;

    if(saved_in < 0 || saved_out < 0 || saved_err < 0){
        perror("dup");
    } else if(apply_redirections(cmd) == 0){
        val = execute_int(cmd);
    }

    fflush(stdout); /* esvaziar o buffer ANTES de restaurar os descritores */

    if(saved_in >= 0){ dup2(saved_in, STDIN_FILENO); close(saved_in); }
    if(saved_out >= 0){ dup2(saved_out, STDOUT_FILENO); close(saved_out); }
    if(saved_err >= 0){ dup2(saved_err, STDERR_FILENO); close(saved_err); }

    return val;
}

int execute_commands(void){
    if(command_list == NULL || command_list->name == NULL){
        return 0;
    }
    
    if(command_list->next == NULL){
        if(is_builtin(command_list->name)){
            return execute_builtin_redir(command_list) == -1 ? -1 : 0;
        }
        execute_ext(command_list);
        return 0;
    }

    return execute_pipe();
}
