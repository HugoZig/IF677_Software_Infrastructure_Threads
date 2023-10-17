/*2. Você deverá implementar um sistema computacional de controle de  ferrovias usando C e pThreads. A ferrovia é composta por 5 interseções, as quais só 
permitem 2 trens simultaneamente.  Todavia, um trem passando em uma interseção não deve afetar (bloquear) o andamento dos outros trens em outras interseções.
 Uma interseção é representada  por uma variável inteira. Um trem passando pela interseção (com menos de 2 trens) deverá modificar a variável contador da 
 interseção indicando a quantidade de trens nesta, e esperar 500 milissegundos para indicar o término de sua passagem. Ao concluir a passagem na interseção, 
 deverá decrementar o respectivo contador em uma unidade. O trem, que liberar uma interseção, somente deverá notificar algum trem aguardando a liberação 
 desta interseção. 

Por exemplo: Trem 1 e Trem 2 estão na interseção 5, e Trem 3 está aguardando a liberação desta interseção (depois de ter passado pela interseção 4). 
Trem 1, ao sair da interseção 5, disponibilizará um trilho na interseção, mas só deverá notificar sua saída para o Trem 3 (pois é o único trem aguardando  
a interseção 5). Os trens deverão ser implementados usando threads, as quais precisam acessar as interseções na sequência 1,2,3,4,5. Ao concluir o percurso,
 cada trem começará a trafegar novamente a partir do início (ou seja, a partir da interseção 1). Assuma a existência de 10 trens.

*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct{
    int id; //identifica o trem para o printf
    int intersecao; //indica a intersecao que o trem planeja entrar
} infoTrem;

int intersecao[5]={0}; //cada intersecao sera uma posicao de 0 a 4 no array
pthread_mutex_t mutexes[5];
pthread_t t[10];

void* AcessarIntersecao(infoTrem *info)
{
    int atual, flag=0; //a flag nos ajuda a ver se o trem entrou ou nao na intersecao
    while(1){
        atual = info->intersecao; 
        /*usamos essa variavel pq incrementamos info->intersecao dentro do primeiro if, 
        e precisamos desse valor constante durante o loop para o trem entrar e sair da 
        intersecao certa, sem travar outros mutexes*/

        pthread_mutex_lock(&mutexes[atual]);
        if(intersecao[atual]<2){
            intersecao[atual] ++; //aumentamos a lotacao da intersecao

            info->intersecao ++; //dizemos que o trem agora pretende ir para a prox intersecao
            if(info->intersecao > 4) info->intersecao = 0; //retorna ao comeco caso ja tenha concluido o percurso
            
            printf("trem %d esta na intersecao %d\nlotacao: %d\n", info->id, atual+1, intersecao[atual]);
            flag=1;
        }
        pthread_mutex_unlock(&mutexes[atual]);

        if(flag){
            sleep(0.5); //espera os 500 milissegundos fora da exclusao mutua
            pthread_mutex_lock(&mutexes[atual]);
            intersecao[atual] --;
            pthread_mutex_unlock(&mutexes[atual]);
            flag=0;
        }

        //else printf("trem %d esta esperando\n", info->id);


    }
}

int main ()
{
    infoTrem trens[10];
    int i;

    for(i=0; i<10; i++){
        trens[i].intersecao = 0;
        trens[i].id=i+1;
    }

    for(i=0; i<5; i++){
        pthread_mutex_init(&(mutexes[i]), NULL);
    }

    for(i=0; i<10; i++){
        if(pthread_create(&(t[i]), NULL, (void*)&AcessarIntersecao, &trens[i])!=0){
            printf("Erro criando thread\n");
            exit (1);
        }
    }

    for(i=0; i<10; i++){
        if(pthread_join(t[i], NULL) != 0){
            printf("Erro executando a função de espera\n");
            exit (1);
        }
    }


    for(i=0; i<5; i++){
        pthread_mutex_destroy(&(mutexes[i]));
    }

    return 0;
}