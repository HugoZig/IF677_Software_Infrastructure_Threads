/*1. Uma cidade começou a usar um novo sistema eletrônico de votação para as eleições municipais. Após o período de votação, os votos de cada bairro estão 
em um arquivo e precisam agora ser  contabilizados. Faça um programa que receba um número N de arquivos , um número T <= N de threads utilizadas para fazer 
a contagem, e um número C de candidatos a prefeito.  Em seguida, o programa deverá abrir os N arquivos nomeados “x.in” no qual 1 <= x <= N. Cada arquivo 
terá 1 voto por linha que será um número y | 0 <= y <= C em que 0 significa voto em branco, 1 significa voto ao candidato 1 e assim sucessivamente. 
Cada thread deverá pegar um arquivo. Quando uma thread concluir a leitura de um arquivo, e houver um arquivo ainda não lido, a thread deverá ler algum 
arquivo pendente.  Ao final imprima na tela o total de votos, a porcentagem de votos recebidos por cada candidato (e dos em branco também) e o número do 
candidato vencedor (que será o candidato com mais votos não importando a porcentagem). 

Assumindo o conhecimento prévido da  quantidade  de threads e arquivos, pode-se definir no início do programa quais arquivos a serem tratados por cada 
thread. Uma outra alternativa ler os arquivos sob demanda, a partir do momento que uma thread termina a leitura de um arquivo, pega qualquer outro não lido 
dinamicamente.

Ademais, deve-se garantir a exclusão múltua ao alterar o array que guardará os votos de cada candidato. Uma implementação mais refinada garante a exclusão 
mútua separada para cada posição do array. Mais especificamente, enquanto um voto está sendo contabilizado para um candidato x e modificando o array na 
respectiva posição, uma outra thread pode modificar o array em uma posição y que representação outro candidato. Ou seja, se o array de votos possui tamanho 
10, haverá um outro array de 10 mutex, um para cada posição do vetor de votos. Ao ler um arquivo e detectar o voto para o candidato y, a thread trava o
mutex relativo à posição y, incrementa a quantidade de votos, e destrava o mutex na posição y. Obviamente, se mais de uma thread quiser modificar a mesma 
posição do array  de votos simultaneamente, somente 1 terá acesso, e as outras estarão bloqueadas. O mutex garantirá a exclusão mútua na posição.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

int qtdFiles=0;
int vencedor=0; //indice do prefeito vencedor
int votosVencedor=0; //votos do vencedor

typedef struct{
    char nome[10];
    int lida; //diz se o arquivo ja foi lido ou nao
    long long int *votes; //aponta para o vetor de votos
    pthread_mutex_t *mutex; //aponta para o mutex relacionado ao arquivo
    pthread_mutex_t *Mvotos; //aponta para o vetor de mutexes relacionado aos votos
} arquivo;

void LerArquivo(arquivo file)
{
    FILE* arq = fopen(file.nome, "rt");

    if(arq==NULL){
        printf("Erro abrindo arquivo\n");
        exit (1);
    }

    int candidato = -1;
    
    while(fscanf(arq, "%d", &candidato)!= EOF){
        pthread_mutex_lock(&file.Mvotos[candidato]);
        file.votes[candidato]++;
        if(file.votes[candidato] > votosVencedor) vencedor = candidato;
        pthread_mutex_unlock(&file.Mvotos[candidato]);
    }

    fclose(arq);
    return;
}

void * FindFile(arquivo* arq)
{
    for(int i=0; i<qtdFiles; i++){
        pthread_mutex_lock(arq[i].mutex); //travar mutex --> arq[i].mutex == &mutexes[i]

        if(arq[i].lida){
            arq[i].lida = 0;
            pthread_mutex_unlock(arq[i].mutex); //destrava mutex 
            LerArquivo(arq[i]);
        }

        else pthread_mutex_unlock(arq[i].mutex); //destrava mutex
    }

    //encerrar a thread
    pthread_exit (NULL);
}

int main ()
{
    int i=0, j=0, qtdThreads=0, qtdPrefeitos=0;
    printf("Qual a quantidade de arquivos? ");
    scanf("%d", &qtdFiles);
    printf("Qual a quantidade de threads? ");
    scanf("%d", &qtdThreads);
    printf("Qual a quantidade de prefeitos? ");
    scanf("%d", &qtdPrefeitos);

    pthread_t t[qtdThreads];
    pthread_mutex_t mutexes[qtdFiles]; //criamos mutexes de controle para saber se o arquivo já foi lido ou não
    pthread_mutex_t vet[qtdPrefeitos+1]; //criamos um mutex responsável pela exclusão mútua do vetor de votos

    long long int votos[qtdPrefeitos+1];
    arquivo files[qtdFiles];

    memset(votos, 0, (qtdPrefeitos+1)*sizeof(long long int)); //zerando vetor de votos
    memset(t, 0, (qtdThreads)*sizeof(pthread_t));

    for(i=0; i<qtdFiles; i++){
        sprintf(files[i].nome, "%d.in", i+1);
        files[i].lida = 1;
        files[i].mutex = &mutexes[i];
        files[i].votes = votos;
        files[i].Mvotos = vet;
    }

    for(i=0; i<qtdFiles; i++){
        pthread_mutex_init(&(mutexes[i]), NULL);
    }
    for(i=0; i<qtdPrefeitos+1; i++){
        pthread_mutex_init(&(vet[i]), NULL);
    }

    for(i=0; i<qtdThreads; i++){
        if(pthread_create(&(t[i]), NULL, (void*)&FindFile, files)!=0){
            printf("Erro criando thread\n");
            exit (1);
        }
    }

    for(i=0; i<qtdThreads; i++){
        if(pthread_join(t[i], NULL) != 0){
            printf("Erro executando a função de espera\n");
            exit (1);
        }
    }

    for(i=0; i<qtdFiles; i++){
        pthread_mutex_destroy(&(mutexes[i]));
    }

    for(i=0; i<qtdPrefeitos+1; i++){
        pthread_mutex_destroy(&(vet[i]));
    }

    
    printf("Votos nulos: %lld votos\n",votos[0]);

    for(i=1; i<qtdPrefeitos+1; i++){
        printf("Candidato %d: %lld votos\n", i, votos[i]);
    }

    printf("\n\nVencedor: candidato %d\n\n", vencedor);

    return 0;

}