/*Um sistema gerenciamento de banco de dados (SGBD) comumente precisa lidar com várias operações de leituras e escritas concorrentes. 
Neste contexto, podemos classificar as threads como leitoras e escritoras. Assuma que enquanto o banco de dados está sendo atualizado devido a uma operação 
de escrita (uma escritora), as threads leitoras precisam ser proibidas em realizar leitura no banco de dados. Isso é necessário para evitar que uma leitora 
interrompa uma modificação em progresso ou leia um dado inconsistente ou inválido.

Você deverá implementar um programa usando pthreads, considerando N threads leitoras e M threads escritoras. A base de dados compartilhada (região crítica) 
deverá ser um array, e threads escritoras deverão continuamente (em um laço infinito) escrever no array em qualquer posição. Similarmente, as threads 
leitoras deverão ler dados (de forma contínua)  de  qualquer posição do array.  As seguintes restrições deverão ser implementadas:

As threads leitoras podem simultaneamente acessar a região crítica (array). Ou seja, uma thread leitora não bloqueia outra thread leitora;
Threads escritoras precisam ter acesso exclusivo à região crítica. Ou seja, a manipulação deve ser feita usando exclusão mútua. Ao entrar na região crítica,
uma thread escritora deverá bloquear todas as outras threads escritoras e threads leitoras que desejarem acessar o recurso compartilhado.

Dica: Você deverá usar mutex e variáveis de condição. 
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define N 5 //threads leitoras
#define M 5 //threads escritoras
#define ArraySize 30

pthread_mutex_t mutexArray;
pthread_t leitoras[N], escritoras[M];
pthread_cond_t canWrite;

int readerOn=0, array[ArraySize] = {0};

/*readerOn indicara à escritora, junto com a variavel de condicao, que tem uma thread lendo, para que as threads leitoras nao bloqueem 
umas as outras, mas impeçam as escritoras. Como a escrita acontece com exclusao mutua, n precisamos de outra variavel de condicao para 
indica-la, pois a leitura so acontece apos incrementarmos readOn, e, ao fazer isso, qq thread escritora que assuma o mutex vai ter que 
esperar a leitura finalizar, de modo que a escrita so aconteça quando a leitura terminou e vice versa. */

int randomNumber()
{
    srand(clock()); // a seed da funcao rand sera definido pela clock, que retorna o tempo que passou desde o comeco da execucao

    return (rand()%ArraySize); //valor aleatorio entre 0 e 29, posicoes do array
}

void * escrevendo(int *id)
{
    while(1){
        sleep(0.3); //tenta escrever a cada 0.3 segundo
        int pos = randomNumber();

        pthread_mutex_lock(&mutexArray); //trava acesso ao array

        while(readerOn){ //se tiver alguma thread lendo, espera
            pthread_cond_wait(&canWrite, &mutexArray);
            printf("thread %d esta esperando para escrever\n", *id);
        }

        printf("thread  escritora %d - o valor na posicao %d sera %d\n", *id, pos, array[pos]+1);
        array[pos] ++;
        pthread_mutex_unlock(&mutexArray); //destrava o acesso ao array
    }
}

void * lendo(int *id)
{
    while(1){
        sleep(0.5); //tenta ler a cada 0.5 segundo
        int pos = randomNumber();

        pthread_mutex_lock(&mutexArray);
        readerOn++; //avisa que quer ler
        pthread_mutex_unlock(&mutexArray);

        int valor = array[pos]; //leitoras podem ter acesso simultaneo, mas a escritora n pode agir pois readerOn!=0
        printf("thread leitora %d - o valor na posicao %d eh %d\n", *id, pos, valor);

        pthread_mutex_lock(&mutexArray);
        readerOn--;
        pthread_mutex_unlock(&mutexArray);

        pthread_cond_broadcast(&canWrite);
    }


}

int main ()
{
    int i, id[M+N];

    pthread_mutex_init(&mutexArray, NULL);
    pthread_cond_init(&canWrite, NULL);


    for(i=0; i<M; i++){
        id[i] = i+1;
        if(pthread_create(&escritoras[i], NULL, (void*)&escrevendo, &id[i])!=0){
            printf("Erro criando thread\n");
            exit (1);
        }
    }

    for(i=0; i<N; i++){
        id[i+M] = i+M+1;
        if(pthread_create(&leitoras[i], NULL, (void*)&lendo, &id[i+M])!=0){
            printf("Erro criando thread\n");
            exit (1);  
        }
    }

    for(i=0; i<M; i++){
        if(pthread_join(escritoras[i], NULL) != 0){
            printf("Erro executando a função de espera\n");
            exit (1);
        }
    }

    for(i=0; i<N; i++){
        if(pthread_join(leitoras[i], NULL) != 0){
            printf("Erro executando a função de espera\n");
            exit (1);
        }
    }


    pthread_mutex_destroy(&mutexArray);
    pthread_cond_destroy(&canWrite);


    return 0;
}