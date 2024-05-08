#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

void *executarComando(void *arg) {
    char *comando = (char *)arg;
    
    int status = system(comando);

    if (status == -1) {
        perror("system");
        exit(EXIT_FAILURE);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    int flag = 0;
    int history = 0;
    char comandoSalvo[1024];
    int teveComando =0;

    if(argc == 1){
         while (1) {
        if (flag == 0) {
            printf("ccpo seq>");
        } else if (flag == 1) {
            printf("ccpo par>");
        }

        char comando[1024];

        if (fgets(comando, sizeof(comando), stdin) == NULL) {
            break;
        }

        comando[strcspn(comando, "\n")] = '\0';

        if (strcmp(comando, "exit")==0){
            exit(0);
        }
        
        if (strcmp(comando, "!!")==0){
            history = 1;
            if(teveComando != 0){
            printf("%s\n", comandoSalvo);
            strcpy(comando, comandoSalvo);
            
            }else {
                printf("No commands\n");
            }
        } else {

            strncpy(comandoSalvo, comando, sizeof(comandoSalvo) -1 );
            comandoSalvo[sizeof(comandoSalvo) - 1] = '\0';
            teveComando = 1;
        }
        

        if (strcmp(comando, "style sequential") == 0) {
            flag = 0;
            continue;
        } else if (strcmp(comando, "style parallel") == 0) {
            flag = 1;
            continue;
        }

        char *token = strtok(comando, ";");
        int flagPipe = 0;
        
        while (token != NULL) {
            pthread_t threads[15];

            if (flag == 0) {
                if (flagPipe == 1) { //pipe
                    char *currentToken = token;
                    char *arrayPipe[15];
                    int contadorPipe = 0;

                    char *tokenPipe = strtok(currentToken, "|");

                    while (tokenPipe != NULL) {
                        arrayPipe[contadorPipe++] = tokenPipe;
                        tokenPipe = strtok(NULL, "|");
                    }

                    int arrayDoPipe[2];

                    if (pipe(arrayDoPipe) == -1) {
                        perror("pipe");
                        exit(EXIT_FAILURE);
                    } 

                    pid_t processoPipe1;
                    pid_t processoPipe2;

                    processoPipe1 = fork(); 
                    if (processoPipe1 == 0) {
                        close(arrayDoPipe[0]);
                        dup2(arrayDoPipe[1], STDOUT_FILENO);
                        close(arrayDoPipe[1]);

                        char *parteDoComando = strtok(arrayPipe[0], " ");
                        char *array2[15];
                        int contador2 = 0;
                    
                        while (parteDoComando != NULL) {
                            array2[contador2++] = parteDoComando;
                            parteDoComando = strtok(NULL, " ");
                        }

                        array2[contador2] = NULL;

                        execvp(array2[0], array2);
                        perror("execvp");
                        exit(EXIT_FAILURE);
                    } else {
                        processoPipe2 = fork();
                        if (processoPipe2 == 0) {
                            close(arrayDoPipe[1]);
                            dup2(arrayDoPipe[0], STDIN_FILENO);
                            close(arrayDoPipe[0]);

                            char *parteDoComando = strtok(arrayPipe[1], " ");
                            char *array2[15];
                            int contador2 = 0;
                    
                            while (parteDoComando != NULL) {
                                array2[contador2++] = parteDoComando;
                                parteDoComando = strtok(NULL, " ");
                            }

                            array2[contador2] = NULL;

                            execvp(array2[0], array2);
                            perror("execvp");
                            exit(EXIT_FAILURE);
                        } else {
                            close(arrayDoPipe[0]);
                            close(arrayDoPipe[1]);
                            waitpid(processoPipe1, NULL, 0);
                            waitpid(processoPipe2, NULL, 0);
                        }
                    }
                } else {
                    pid_t processo1 = fork();

                    if (processo1 == -1) {
                        perror("fork");
                        exit(EXIT_FAILURE);
                    } else if (processo1 == 0) { 
                        char *parteDoComando = strtok(token, " ");
                        char *array2[15];
                        int contador2 = 0;

                        while (parteDoComando != NULL) {
                            array2[contador2++] = parteDoComando;
                            parteDoComando = strtok(NULL, " ");
                        }

                        array2[contador2] = NULL;

                        execvp(array2[0], array2);
                        perror("execvp");
                        exit(EXIT_FAILURE);
                    } else {
                        wait(NULL);
                    }
                }
            } else if (flag == 1) {
                if (flagPipe == 1) {
                    char *currentToken = token;
                    char *arrayPipe[15];
                    int contadorPipe = 0;

                    char *tokenPipe = strtok(currentToken, "|");

                    while (tokenPipe != NULL) {
                        arrayPipe[contadorPipe++] = tokenPipe;
                        tokenPipe = strtok(NULL, "|");
                    }

                    for (int i = 0; i < contadorPipe; i++) {
                        if (pthread_create(&threads[i], NULL, executarComando, (void *)arrayPipe[i]) != 0) {
                            perror("pthread_create");
                            exit(EXIT_FAILURE);
                        }
                    }

                    for (int i = 0; i < contadorPipe; i++) {
                        pthread_join(threads[i], NULL);
                    }
                } else {
                    pthread_t thread;
                    if (pthread_create(&thread, NULL, executarComando, (void *)token) != 0) {
                        perror("pthread_create");
                        exit(EXIT_FAILURE);
                    }

                    pthread_join(thread, NULL);
                }
            }

            token = strtok(NULL, ";");
        }
    }

    return 0;
    } else if(argc == 2){
        FILE *file = fopen(argv[1], "r");
         while (1) {
        if (flag == 0) {
            printf("ccpo seq>");
        } else if (flag == 1) {
            printf("ccpo par>");
        }

        char comando[1024];

        if (fgets(comando, sizeof(comando), file) == NULL) {
            break;
        } 
        comando[strcspn(comando, "\n")] = '\0';

        if (strcmp(comando, "exit")==0){
        
            exit(0);
        }

         if (strcmp(comando, "!!")==0){
            history = 1;
            if(teveComando != 0){
            printf("%s\n", comandoSalvo);
            strcpy(comando, comandoSalvo);
            
            }else {
                printf("No commands\n");
            }
        } else {

            strncpy(comandoSalvo, comando, sizeof(comandoSalvo) -1 );
            comandoSalvo[sizeof(comandoSalvo) - 1] = '\0';
            teveComando = 1;
        }

        if (strcmp(comando, "style sequential") == 0) {
            flag = 0;
            continue;
        } else if (strcmp(comando, "style parallel") == 0) {
            flag = 1;
            continue;
        }

        if(strcmp(comando, "!!")== 0){
            history=1;
        }

        char *token = strtok(comando, ";");
        int flagPipe = 0;
        
        while (token != NULL) {
            pthread_t threads[15];

            if (flag == 0) {
                if (flagPipe == 1) {
                    char *currentToken = token;
                    char *arrayPipe[15];
                    int contadorPipe = 0;

                    char *tokenPipe = strtok(currentToken, "|");

                    while (tokenPipe != NULL) {
                        arrayPipe[contadorPipe++] = tokenPipe;
                        tokenPipe = strtok(NULL, "|");
                    }

                    int arrayDoPipe[2];

                    if (pipe(arrayDoPipe) == -1) {
                        perror("pipe");
                        exit(EXIT_FAILURE);
                    } 

                    pid_t processoPipe1;
                    pid_t processoPipe2;

                    processoPipe1 = fork(); 
                    if (processoPipe1 == 0) {
                        close(arrayDoPipe[0]);
                        dup2(arrayDoPipe[1], STDOUT_FILENO);
                        close(arrayDoPipe[1]);

                        char *parteDoComando = strtok(arrayPipe[0], " ");
                        char *array2[15];
                        int contador2 = 0;
                    
                        while (parteDoComando != NULL) {
                            array2[contador2++] = parteDoComando;
                            parteDoComando = strtok(NULL, " ");
                        }

                        array2[contador2] = NULL;

                        execvp(array2[0], array2);
                        perror("execvp");
                        exit(EXIT_FAILURE);
                    } else {
                        processoPipe2 = fork();
                        if (processoPipe2 == 0) {
                            close(arrayDoPipe[1]);
                            dup2(arrayDoPipe[0], STDIN_FILENO);
                            close(arrayDoPipe[0]);

                            char *parteDoComando = strtok(arrayPipe[1], " ");
                            char *array2[15];
                            int contador2 = 0;
                    
                            while (parteDoComando != NULL) {
                                array2[contador2++] = parteDoComando;
                                parteDoComando = strtok(NULL, " ");
                            }

                            array2[contador2] = NULL;

                            execvp(array2[0], array2);
                            perror("execvp");
                            exit(EXIT_FAILURE);
                        } else {
                            close(arrayDoPipe[0]);
                            close(arrayDoPipe[1]);
                            waitpid(processoPipe1, NULL, 0);
                            waitpid(processoPipe2, NULL, 0);
                        }
                    }
                } else {
                    pid_t processo1 = fork();

                    if (processo1 == -1) {
                        perror("fork");
                        exit(EXIT_FAILURE);
                    } else if (processo1 == 0) {
                        char *parteDoComando = strtok(token, " ");
                        char *array2[15];
                        int contador2 = 0;

                        while (parteDoComando != NULL) {
                            array2[contador2++] = parteDoComando;
                            parteDoComando = strtok(NULL, " ");
                        }

                        array2[contador2] = NULL;

                        execvp(array2[0], array2);
                        perror("execvp");
                        exit(EXIT_FAILURE);
                    } else {
                        wait(NULL);
                    }
                }
            } else if (flag == 1) {
                if (flagPipe == 1) {
                    char *currentToken = token;
                    char *arrayPipe[15];
                    int contadorPipe = 0;

                    char *tokenPipe = strtok(currentToken, "|");

                    while (tokenPipe != NULL) {
                        arrayPipe[contadorPipe++] = tokenPipe;
                        tokenPipe = strtok(NULL, "|");
                    }

                    for (int i = 0; i < contadorPipe; i++) {
                        if (pthread_create(&threads[i], NULL, executarComando, (void *)arrayPipe[i]) != 0) {
                            perror("pthread_create");
                            exit(EXIT_FAILURE);
                        }
                    }

                    for (int i = 0; i < contadorPipe; i++) {
                        pthread_join(threads[i], NULL);
                    }
                } else {
                    pthread_t thread;
                    if (pthread_create(&thread, NULL, executarComando, (void *)token) != 0) {
                        perror("pthread_create");
                        exit(EXIT_FAILURE);
                    }

                    pthread_join(thread, NULL);
                }
            }

            token = strtok(NULL, ";");
        }
    }

    return 0;
    }
}

   