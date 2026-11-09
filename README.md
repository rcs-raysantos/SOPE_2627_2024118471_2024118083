# Minishell em C — Sistemas Operativos

## 1. Descrição do Projeto

Este projeto consiste no desenvolvimento de uma **Minishell em C**, executada num ambiente Linux/POSIX, no âmbito da unidade curricular de Sistemas Operativos.

O objetivo é implementar uma shell simplificada capaz de interpretar e executar comandos, gerir processos, realizar redirecionamentos de entrada e saída, permitir a comunicação entre processos através de pipes e executar tarefas concorrentes com recurso a threads e semáforos POSIX.

O projeto está dividido em duas fases, cada uma correspondendo a 50% da avaliação final.

## 2. Objetivos

- Compreender o ciclo de vida dos processos em sistemas operativos.
- Utilizar `fork()`, `execvp()` e `waitpid()` para criar, executar e gerir processos.
- Implementar comandos internos (*built-ins*).
- Executar comandos em primeiro plano e em segundo plano (*background*).
- Redirecionar a entrada, a saída e os erros através de descritores de ficheiros.
- Implementar pipelines utilizando pipes POSIX.
- Gerir sinais e evitar processos zombie.
- Desenvolver aplicações multithreaded com a biblioteca POSIX Threads.
- Sincronizar o acesso a recursos partilhados através de semáforos.
- Implementar um modelo produtor-consumidor com um buffer circular limitado.
- Garantir a gestão correta da memória e o tratamento de erros.

## 3. Tecnologias Utilizadas

- **Linguagem:** C (ANSI C / POSIX).
- **Sistema operativo:** Linux.
- **Compilador:** GCC.
- **Concorrência:** POSIX Threads (`pthread`).
- **Sincronização:** Semáforos POSIX (`sem_t`).
- **Gestão de processos:** `fork()`, `execvp()`, `waitpid()`.
- **Comunicação entre processos:** `pipe()`.
- **Descritores de ficheiros:** `open()`, `close()`, `dup2()`.
- **Sistema de ficheiros:** `opendir()`, `readdir()`, `stat()`.

## 4. Requisitos do Projeto

### Fase 1 — Shell, Processos e Pipes

#### 4.1. Ciclo principal da shell (REPL)

Implementação de um ciclo *Read-Eval-Print Loop* que apresenta um prompt, recebe comandos do utilizador, utiliza o parser fornecido pelo docente e executa os comandos resultantes.

Deve também garantir a libertação da memória dinâmica utilizada pelas estruturas de dados.

#### 4.2. Comandos externos e internos

A shell deverá executar comandos externos através de processos filhos e disponibilizar os seguintes comandos internos:

- `cd <diretoria>` — altera a diretoria atual.
- `pwd` — apresenta o caminho absoluto da diretoria atual.
- `exit` — termina a shell.
- `history` — apresenta o histórico de comandos.
- `echo <texto>` — apresenta texto no terminal.
- `help` — apresenta informação sobre os comandos disponíveis.

Os comandos internos devem ser executados no processo da própria shell quando necessário, de modo a preservar o estado da sessão.

#### 4.3. Execução em background

A shell deverá permitir executar comandos em segundo plano através do operador `&`.

O utilizador deverá recuperar imediatamente o prompt, sem esperar pelo fim do processo, e deverá ser apresentado o PID do processo iniciado.

#### 4.4. Redirecionamento de entrada e saída

Devem ser suportados os seguintes operadores:

- `>` — redireciona a saída padrão para um ficheiro, substituindo o conteúdo existente.
- `>>` — acrescenta a saída ao final de um ficheiro.
- `<` — utiliza um ficheiro como entrada padrão.
- `2>` — redireciona a saída de erro padrão para um ficheiro.

A implementação deverá utilizar as chamadas de sistema apropriadas para abrir ficheiros e substituir os descritores padrão.

#### 4.5. Pipes e gestão de sinais

A shell deverá suportar pipelines com vários comandos, permitindo que a saída de um processo seja utilizada como entrada do seguinte.

Exemplo:

`ls -l | grep ".c" | wc -l`

Também deverá gerir os processos em background que terminem, evitando a acumulação de processos zombie, e proteger a shell contra interrupções ou suspensões provocadas por `Ctrl+C` e `Ctrl+Z`.

### Fase 2 — Threads, Concorrência e Sincronização

#### 4.6. Comando `indexer`

Implementação do comando:

`indexer <diretoria> [-t num_threads]`

O comando deverá percorrer recursivamente a árvore de diretorias e contabilizar:

- Número total de ficheiros.
- Número total de diretorias.
- Tamanho total dos ficheiros em bytes.
- Número de ficheiros por extensão.

O processamento deverá utilizar threads de trabalho e apresentar um relatório consolidado no final.

#### 4.7. Sincronização do `indexer`

As estruturas partilhadas utilizadas pelas threads deverão ser protegidas com mecanismos POSIX de sincronização, de forma a evitar condições de corrida (*race conditions*) e garantir resultados consistentes.

#### 4.8. Comando `pbackup`

Implementação do comando:

`pbackup <origem> <destino> -j <num_threads>`

O comando deverá realizar cópias de segurança paralelas através de um modelo produtor-consumidor.

A thread produtora percorre a diretoria de origem e coloca os caminhos dos ficheiros num buffer circular limitado. As threads consumidoras retiram esses caminhos e copiam os ficheiros para o destino através de operações de leitura e escrita.

#### 4.9. Sincronização do buffer circular

O buffer circular deverá utilizar semáforos e mecanismos de exclusão mútua para controlar as posições disponíveis e ocupadas.

A implementação deverá garantir:

- Proteção dos índices de inserção e remoção.
- Bloqueio do produtor quando o buffer estiver cheio.
- Bloqueio dos consumidores quando o buffer estiver vazio.
- Coordenação da terminação das threads consumidoras.
- Encerramento correto do processo de backup.

## 5. Compilação e Execução

A compilação deverá ser realizada com GCC, utilizando as opções exigidas no enunciado:

```bash
gcc -Wall -Wextra -pedantic -pthread -std=c11 *.c -o minishell
```

**Nota:** o comando acima é um exemplo genérico. Deve ser adaptado aos ficheiros do projeto e ao módulo de parsing fornecido pelo docente.

Após a compilação, caso o executável tenha o nome `minishell`, poderá ser iniciado com:

```bash
./minishell
```

## 6. Testes e Validação

A implementação deverá ser testada para verificar:

- Execução correta de comandos externos e internos.
- Funcionamento de comandos em background.
- Redirecionamentos de entrada, saída e erro.
- Execução de pipelines com vários processos.
- Gestão de sinais e recolha de processos terminados.
- Contabilização correta dos ficheiros e diretorias pelo `indexer`.
- Funcionamento concorrente do `pbackup`.
- Ausência de fugas de memória.
- Tratamento de erros nas chamadas de sistema.
- Compilação sem warnings.

O Valgrind poderá ser utilizado para verificar a gestão de memória, conforme indicado no enunciado.

## 7. Organização do Trabalho

O desenvolvimento deverá ser organizado por requisitos, permitindo acompanhar a implementação, os testes e a validação de cada funcionalidade através dos GitHub Issues.

## 8. Estado do Projeto

O estado de implementação de cada requisito deverá ser atualizado à medida que o trabalho progride. A existência de um requisito neste documento representa uma especificação do projeto, não uma confirmação de que a funcionalidade já está implementada.

## 9. Unidade Curricular

**Unidade curricular:** Sistemas Operativos  
**Projeto:** Minishell em C  
**Linguagem:** C (ANSI C / POSIX)  
**Avaliação:** Fase 1 — 50% | Fase 2 — 50%

## Objetivos Minishell para a primeira entrega:

- **Eu trato dos requisitos 1.1 e 1.2:** ciclo principal da shell (REPL), integração com o parser, limpeza de memória, execução de comandos externos e implementação dos comandos internos.
- **Tu tratas dos requisitos 1.3 e 1.4:** execução de comandos em background, sem bloquear o prompt, apresentação do PID e redirecionamentos de entrada, saída e erro (`>`, `>>`, `<` e `2>`).
- **No requisito 1.5, dividimos assim:** eu implemento os pipes (`|`) entre comandos; tu tratas dos sinais, incluindo `SIGCHLD` com `WNOHANG` para evitar processos zombie e a proteção da shell contra `SIGINT` (Ctrl+C) e `SIGTSTP` (Ctrl+Z).

Vamos combinar as interfaces entre as partes — sobretudo como o parser indica background, redirecionamentos e pipes — antes de integrar, para garantir que os módulos funcionam juntos.