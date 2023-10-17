#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>


#define lado 2  //Define-se aqui o lado da matriz do input
#define Existe 1    //Define Existe = 1
#define nExiste 0   //Define nExiste = 0


int matrix[lado][lado] = {};    //matriz


bool line[lado][125] = { 0 };   //Para cada linha, temos um array de booleanos (iniciados em zero) do tamanho da tablea ascii. Caso o elemento esteja na linha, colocamos como 1 na posicao do seu valor

bool column[lado][125] = { 0 };     //Para cada coluna, temos um array de booleanos (iniciados em zero) do tamanho da tablea ascii. Caso o elemento esteja na linha, colocamos como 1 na posicao do seu valor


void* checkL(void* linha);  //Funcao que checa se ha repeticao de simbolos nas linhas


void* checkC(void* coluna);  //Funcao que checa se ha repeticao de simbolos nas colunas



int main(void){
    int i, j;
    char word;
    bool flag = false;
    bool** answer = (bool**) calloc(lado, sizeof(bool*));
    //Iniciando ponteiro de ponteiro de booleano
    if (answer == NULL){
           printf("error at contiguous memory allocation in main\n");
           exit (1);
        }
    pthread_t threads[lado];    //Array de threads
    
    for (i = 0; i < lado; i++) {        //Leitura Matriz em convertendo char para inteiro para utilizar tabela ascii
        for (j = 0; j < lado; j++) {
            scanf(" %c", &word);
            matrix[i][j] = word;
        }
    }
    
    for (i = 0; i < lado; i++) {    // Inicializando Threads para checar as linhas
        int* holder = (int*) malloc(sizeof(int));
        if (holder == NULL){
           printf("error at memory allocation in main\n");
           exit (2);
        }
        *holder = i;    //passa-se o numero da linha como parametetro
        if (pthread_create(&threads[i], NULL, &checkL, holder) != 0) {
            printf("error at thread creation\n");
            exit(3);
        }
    }
    for (i = 0; i < lado; i++) {    // Dando join nas Threads que checaram as linhas
        if (pthread_join(threads[i], (void**) &answer[i]) != 0) {   //recebe retorno da funcao no ponteiro de bool
            printf("error at thread join\n");
            exit(4);
        }
    }
    for (i = 0; i < lado; i++) {
        if (*answer[i] == Existe) {     //Se houver alguma repeticao de simbolo, ativa flag
            flag = Existe;
        }
        free(answer[i]);    //Free no ponteiro
    }
    if (flag) {     //Caso flag ativa, nao eh um quadrado latino
        printf("A matriz não é um quadrado latino\n");
    }
    else{   //Se flag nao tiver ativa, checa colunas
        for (i = 0; i < lado; i++) {    // Inicializando Threads para checar as colunas
            int* holder = (int*) malloc(sizeof(int));
            if (holder == NULL){
                printf("error at contiguous memory allocation in main\n");
                exit (5);
            }
            *holder = i;        //passa-se o numero da coluna como parametetro
            if (pthread_create(&threads[i], NULL, &checkC, holder) != 0) {
                printf("error at thread creation\n");
                exit(6);
            }
        }
        for (i = 0; i < lado; i++) {    // Dando join nas Threads que checaram as linhas
            if (pthread_join(threads[i], (void**) &answer[i]) != 0) {   //recebe retorno da funcao no ponteiro de bool
                printf("error at thread join\n");
                exit(7);
            }
        }
        for (i = 0; i < lado; i++) {
            if (*answer[i] == Existe) {   //Se houver alguma repeticao de simbolo, ativa flag
                flag = Existe;
            }
            free(answer[i]);    //Free no ponteiro
        }
        
        if (flag) {     //Se a flag estiver ativa, nao eh um quadrado latino
            printf("A matriz não é um quadrado latino\n");
        }
        else printf("A matriz é um quadrado latino\n");     //Se a flag nao estiver ativa, eh um quadrado latino
    }
    free(answer);   //Free no ponteiro de ponteiro
}

void* checkL(void* linha){  //Funcao que checa se ha repeticao de simbolos nas linhas
    int i;
    bool* rsp = (bool*) malloc(sizeof(bool));       //bool de retorno: 1 = tem dois ou mais simbolos iguais; 0 = nao ha repeticao
    if (rsp == NULL){
           printf("error at memory allocation in checkL\n");
           exit (8);
        }
    int val = *(int*)linha;
    for (i = 0; i < lado; i++) {
        if (line[val][matrix[val][i]] == nExiste) {     //caso o elemento nao exista no array, adicione-o, caso contrario, ha repeticao
            line[val][matrix[val][i]] = Existe;
        }
        else {
            *rsp = 1;   //ha repeticao, logo retorna 1
            free(linha);
            return (void*) rsp;
            
        }
    }
    *rsp = 0;   //nao ha repeticao, logo retorna 0
    free(linha);    //Free no parametro passado
    return (void*) rsp;
}


void* checkC(void* coluna){
    int i;
    bool* rsp = (bool*) malloc(sizeof(bool));
    if (rsp == NULL){
           printf("error at memory allocation in checkC\n");
           exit (9);
        }
    int val = *(int*)coluna;        //bool de retorno: 1 = tem dois ou mais simbolos iguais; 0 = nao ha repeticao
    for (i = 0; i < lado; i++) {
        if (column[val][matrix[i][val]] == nExiste) {      //caso o elemento nao exista no array, adicione-o, caso contrario, ha repeticao
            column[val][matrix[i][val]] = Existe;
        }
        else {
            *rsp = 1;   //ha repeticao, logo retorna 1
            free(coluna);
            return (void*) rsp;
            
        }
    }
    *rsp = 0;   //nao ha repeticao, logo retorna 0
    free(coluna);   //Free no parametro passado
    return (void*) rsp;
}
