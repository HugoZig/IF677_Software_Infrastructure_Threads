#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#define nlinhas 4
#define ncolunas 3

typedef struct{
    int rowpos;
    float value;
} elemento;

typedef struct{
    elemento *nonzeros;
    int qtd;
} vetor;                //vai representar uma linha da matriz

typedef struct{
    vetor *listas;
} sparsematrix;

typedef struct{
    int linha;
    float *dense;
} parametros_vetor;     //parametros para multiplicar por vetor

typedef struct{
    int posicao;
    int colunas;
    float **dense;
} parametros_denso;    //parametros da matriz densa

typedef struct{
    int posicao;
    int colunas;
    sparsematrix operante;
} parametros_sparse;     //parametros multiplicacao de esparsas

sparsematrix matrizinicial;
sparsematrix matrizresultante;

void initiate(sparsematrix *matriz, int linhas, int colunas){ //iniciar a matriz esparsa
    int i;
    //printf("%d colunas\n", colunas);
    matriz->listas = (vetor*) malloc (sizeof(vetor) * linhas);
    if(matriz->listas == NULL){
        printf("erro de alocacao...");
        exit(1);
    }
    for(i=0; i< linhas; i++){
        matriz->listas[i].qtd = 0;
        matriz->listas[i].nonzeros = NULL;
    }
}

void printsparsematrix(sparsematrix *matriz, int linhas){ //mostrar a matriz esparsa
    int i, j;
    printf("{");
    for(i=0; i< linhas; i++){
        printf("{");
        //printf("a linha tem %d numeros", matriz->listas[i].qtd);
        for(j=0; j< matriz->listas[i].qtd; j++){
            printf("(%d,%.1f)", matriz->listas[i].nonzeros[j].rowpos, matriz->listas[i].nonzeros[j].value);
            if(j + 1 < matriz->listas[i].qtd) printf(",");
        }
        printf("}");
        if(i+1 < linhas) printf(",\n");
    }
    printf("}\n");
}

void insert(sparsematrix *matriz, int row, int column, float value){ //inserir na matriz
    int i = matriz->listas[row].qtd;
    matriz->listas[row].qtd ++;
    matriz->listas[row].nonzeros = (elemento*) realloc(matriz->listas[row].nonzeros, matriz->listas[row].qtd * sizeof(elemento));
    if(matriz->listas[row].nonzeros == NULL){
        printf("erro de alocacao...");
        exit(1);
    }
    matriz->listas[row].nonzeros[i].value = value;
    matriz->listas[row].nonzeros[i].rowpos = column;
}

void ler(sparsematrix *matriz){ //ler os numeros da matriz
    int i, numbers, row, column;
    float value;
    printf("Quantos numeros diferentes de zero ha na matriz?\n");
    scanf("%d", &numbers);
    printf("Digite linha coluna valor\n");
    for(i=0; i<numbers; i++){
        scanf("%d %d %f", &row, &column, &value);
        if(row >= nlinhas || row < 0){
            printf("Linha invalida, tente de novo\n");
            i--;
            continue;
        }
        if(column >= ncolunas || column < 0){
            printf("Coluna invalida, tente de novo\n");
            i--;
            continue;
        }
        if(value == 0){
            printf("Valor invalido, tente de novo\n");
            i--;
            continue;
        }
        insert(matriz, row, column, value);
    }
}

void leralternativo(sparsematrix *matriz, int limite){ //trocar linha por coluna da insercao normal
    int i, numbers, row, column;
    float value;
    printf("Quantos numeros diferentes de zero ha na matriz?\n");
    scanf("%d", &numbers);
    printf("Digite linha coluna valor\n");
    for(i=0; i<numbers; i++){
        scanf("%d %d %f", &row, &column, &value);
        if(row >= ncolunas || row < 0){
            printf("Linha invalida, tente de novo\n");
            i--;
            continue;
        }
        if(column >= limite || column < 0){
            printf("Coluna invalida, tente de novo\n");
            i--;
            continue;
        }
        if(value == 0){
            printf("Valor invalido, tente de novo\n");
            i--;
            continue;
        }
        insert(matriz, column, row, value);
    }
}

float find(sparsematrix *matriz, int row, int position){  //conferir se ha um numero na linha row na posicao i
    int i;
    for(i=0; i< matriz->listas[row].qtd; i++)
        if(matriz->listas[row].nonzeros[i].rowpos == position) return matriz->listas[row].nonzeros[i].value;
    return 0;
}

void *mulpartvetor(void* par){ //cada thread calcula um numero
    parametros_vetor para = (*(parametros_vetor*)par);
    float result = 0;
    int i, tmp;
    for(i = 0; i<ncolunas; i++){
        tmp = find(&matrizinicial, para.linha, i);
        if(tmp) result += para.dense[i] * tmp;
    }
    //printf("%f %d\n", result[linha], linha);
    if(result) insert(&matrizresultante, para.linha, 0, result);
    if(par != NULL) free(par);
    pthread_exit(NULL);
}

void mulvetor(void){ //vetor tura um coluna e um numero de linhas ja definido
    printf("Escreva %d valores para o vetor:\n", ncolunas);
    initiate(&matrizresultante, nlinhas, 1);
    int i;
    float *dense;
    pthread_t threads[nlinhas];
    parametros_vetor *para[nlinhas];
    parametros_vetor parametros;
    
    dense = (float*) malloc (sizeof(float) * ncolunas);
    if(dense == NULL){
        printf("erro de alocacao...");
        exit(1);
    }
    for(i = 0; i< ncolunas; i++){   //ler vetor
        scanf("%f", &dense[i]);
    }
    parametros.dense = dense;
    for(i=0; i< nlinhas; i++){
        parametros.linha = i;
        para[i] = (parametros_vetor*) malloc (sizeof(parametros_vetor));
        if(para[i] == NULL){
            printf("erro de alocacao...");
            exit(1);
        }
        *para[i] = parametros;
        if(pthread_create(&threads[i],NULL, mulpartvetor, (void *)para[i])){
            printf("Erro de criacao...");
            exit(2);
        }
    }
    for(i=0; i< nlinhas; i++){
        if(pthread_join(threads[i], NULL)){
            printf("Erro no join...");
            exit(3);
        }
    }
    printsparsematrix(&matrizresultante, nlinhas);
    if(dense != NULL) free(dense);
    for(i=0; i< nlinhas; i++)
        if(matrizresultante.listas[i].nonzeros != NULL) free(matrizresultante.listas[i].nonzeros);
    if(matrizresultante.listas != NULL) free(matrizresultante.listas);
}

elemento find_elemento(sparsematrix *matriz, int linha, int inicio){ //achar um elemento => (valor + posicao) na matriz
    if(inicio < matriz->listas[linha].qtd)
       return matriz->listas[linha].nonzeros[inicio];
    else{
        elemento e;
        e.rowpos = -1;
        return e;
    }
}

void* mulpartmatrix(void* arg){ //cada thread calcula a linha de uma matriz final
    float *result, tmp;
    int i, j;
    elemento element;
    parametros_denso par = (*(parametros_denso*)arg);
    result = (float*) calloc (sizeof(float), par.colunas);
    if(result == NULL){
        printf("Erro de alocacao...");
        exit(1);
    }
    for(i = 0; i< matrizinicial.listas[par.posicao].qtd; i++){
        element = find_elemento(&matrizinicial, par.posicao, i); //um elemento e pego na matriz esparsa
        for(j = 0; j< par.colunas; j++){
            tmp = par.dense[j][element.rowpos];  //para cada elemto da linha resultante, e adicionado o valor da esparsa vezes os valores da matriz cuja posicao de linha seja igual a posicao de coluna do elemento pego
            if(tmp) result[j] += element.value * tmp;
        }
    }
    for(i=0; i<par.colunas; i++)
        if(result[i]) insert(&matrizresultante, par.posicao, i, result[i]);
    if(result != NULL) free(result);
    if(arg != NULL) free(arg);
    pthread_exit(NULL);
}

void muldensematrix(void){   //multiplicar matriz densa
    int colunas, i, j;
    printf("Digite o número de colunas:\n");  //numero de linha ja e defenido
    scanf("%d", &colunas);
    float **dense = NULL;
    dense = (float**) malloc (sizeof(float*) * colunas);
    if(dense == NULL){
        printf("Erro de alocacao");
        exit(4);
    }
    for(i=0; i< colunas; i++){
        dense[i] = (float*) malloc (sizeof(float) * ncolunas);
        if(dense[i] == NULL){
            printf("Erro de alocacao");
            exit(5);
        }
    }
    printf("Digite %d numeros:\n", colunas * ncolunas);
    for(i=0; i< ncolunas; i++)
        for(j=0; j< colunas; j++)
            scanf("%f", &dense[j][i]);   //a transposta e lida ao inves da matriz normal para facilitar a implementacao da multiplicacao
    
    pthread_t threads[nlinhas];
    parametros_denso *para[nlinhas];
    parametros_denso parametros;
    initiate(&matrizresultante, nlinhas, colunas);
    parametros.dense = dense;
    parametros.colunas = colunas;
    
    for(i=0; i< nlinhas; i++){
        parametros.posicao = i;
        para[i] = (parametros_denso*) malloc (sizeof(parametros_denso));
        if(para[i] == NULL){
            printf("Erro de alocacao");
            exit(1);
        }
        *para[i] = parametros;
        if(pthread_create(&threads[i],NULL, mulpartmatrix, (void *)para[i])){
            printf("Erro de criacao");
            exit(2);
        }
    }
    for(i=0; i< nlinhas; i++){
        if(pthread_join(threads[i], NULL)){
            printf("Erro de join");
            printf("%d", i);
        }
    }
    printsparsematrix(&matrizresultante, nlinhas);
    for(i=0; i< colunas; i++)
        if(dense[i] != NULL) free(dense[i]);
    if(dense != NULL) free(dense);
    for(i=0; i< nlinhas; i++)
        if(matrizresultante.listas[i].nonzeros != NULL) free(matrizresultante.listas[i].nonzeros);
    if(matrizresultante.listas != NULL) free(matrizresultante.listas);

}

void* mulpartsparse(void* arg){  //cada thread calcula uma linha da matriz resultante
    float *result;
    float valor;
    elemento auxiliar;
    int i, j;
    parametros_sparse par;
    par = (*(parametros_sparse*) arg);
    result = (float*) calloc(par.colunas, sizeof(float));
    if(result == NULL){
        printf("Erro de alocacao");
        exit(1);
    }
    //("Ha %d numeros nessa linha\n", matrizinicial.listas[par.posicao].qtd);
    for(i=0;i<matrizinicial.listas[par.posicao].qtd;i++){
        //printf("O elemento achado foi ");  //pega um elemento da inical
        auxiliar = find_elemento(&matrizinicial, par.posicao, i);
        //printf("%f, coluna %d\n", auxiliar.value, auxiliar.rowpos);
        for(j=0; j< par.colunas; j++){
            valor = find(&par.operante, j, auxiliar.rowpos);
            result[j] += auxiliar.value * valor;
        }
    }
    for(i=0; i<par.colunas; i++){
        printf("%f %d %d\n", result[i], par.posicao, i);
        if(result[i]) {
            //printf("foi inserido\n");
            insert(&matrizresultante, par.posicao, i, result[i]);
        }
    }
    if(arg != NULL) free(arg);
    if(result != NULL) free(result);
    pthread_exit(NULL);
}

void mulsparsematrix(void){
    sparsematrix operante;
    int colunas, i;
    printf("Quantas colunas?\n");
    scanf("%d", &colunas);
    initiate(&operante, colunas, ncolunas);
    initiate(&matrizresultante, nlinhas, colunas);
    leralternativo(&operante, colunas);  //segunda matriz vai ser lida como a transposta
    printsparsematrix(&operante, colunas);
    
    pthread_t threads[nlinhas];
    parametros_sparse *para[nlinhas];
    parametros_sparse parametros;
    parametros.operante = operante;
    parametros.colunas = colunas;
    
    for(i=0; i< nlinhas; i++){
        parametros.posicao = i;
        para[i] = (parametros_sparse*) malloc (sizeof(parametros_sparse));
        if(para[i] == NULL){
            printf("Erro de alocacao");
            exit(1);
        }
        *para[i] = parametros;
        if(pthread_create(&threads[i],NULL, mulpartsparse, (void *)para[i])){
            printf("Erro de criacao");
            exit(2);
        }
    }
    
    for(i=0; i< nlinhas; i++){
        if(pthread_join(threads[i], NULL)){
            printf("Erro de join");
            exit(3);
        }
    }
    
    printsparsematrix(&matrizresultante, nlinhas);
    
    for(i=0; i< colunas; i++)
        if(operante.listas[i].nonzeros != NULL) free(operante.listas[i].nonzeros);
    if(operante.listas != NULL) free(operante.listas);

    for(i=0; i< nlinhas; i++)
        if(matrizresultante.listas[i].nonzeros != NULL) free(matrizresultante.listas[i].nonzeros);
    if(matrizresultante.listas != NULL) free(matrizresultante.listas);

}

int main(){
    int i = 0;
    initiate(&matrizinicial, nlinhas, ncolunas);
    ler(&matrizinicial);
    printsparsematrix(&matrizinicial, nlinhas);
    printf("Selecione a operacao a ser feita:\n");
    while(abs(i) > 3 || i == 0){
        printf("1...Multiplicar por vetor\n");
        printf("2...Multiplicar por matriz densa\n");
        printf("3...Multiplicar por matriz esparsa\n");
        scanf("%d", &i);
        if(i == 1) mulvetor();
        else if(i == 2)muldensematrix();
        else if(i == 3)mulsparsematrix();
    }
    for(i=0; i<nlinhas; i++)
        if(matrizinicial.listas[i].nonzeros != NULL) free(matrizinicial.listas[i].nonzeros);
    if(matrizinicial.listas != NULL) free(matrizinicial.listas);
    return 0;
}
