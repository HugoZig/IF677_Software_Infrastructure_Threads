#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define comer 3     //Tempo de comer em segundos
#define pensar 3    //Tempo de pensar em segundos

void* filosofos(void* id);  //Funcao principal

void think(int id); //Funcao de pensar

void get_forks(int id); //Funcao de pegar garfos

void eat(int id);   //Funcao de comer

void put_forks(int id); //Funcao de pegar garfo

pthread_mutex_t mutex;  //Mutex

pthread_cond_t cond;    //Variavel de condicao

bool forks[5] = { false };  //Array de booleanos onde cada posicao representa um garfo



int main(){
    int i;
    pthread_t threads[5];   //Array de threads
    pthread_mutex_init(&mutex, NULL);   //Inicializacao do mutex
    pthread_cond_init(&cond, NULL);     //Inicializacao da variavel de condicao
    for (i = 0; i < 5; i++) {   //Inicializando Threads
        int* holder = (int*) malloc(sizeof(int));
        if (holder == NULL){
           printf("error at memory allocation in main\n");
           exit (1);
        }
        *holder = i;
        if (pthread_create(&threads[i], NULL, &filosofos, holder) != 0) {
            printf("error at thread creation\n");
            exit(2);
        }
    }
    for (i = 0; i < 5; i++) {   //Dando Join na threads
        if (pthread_join(threads[i], NULL) != 0) {
            printf("error at thread join\n");
            exit(3);
        }
    }
    pthread_mutex_destroy(&mutex);  //Destruindo Mutex
    pthread_cond_destroy(&cond);    //Destruindo Varivel de condicao
}



void* filosofos(void* id){
    while (true) {      //Loop exemplificado na questao
        think(*(int*)id);
        get_forks(*(int*)id);
        eat(*(int*) id);
        put_forks(*(int*) id);
    }
}


void get_forks(int id){
    pthread_mutex_lock(&mutex); //Acessa a regiao critica
    while (!(forks[(id+1)%5] == 0 && forks[id] == 0)) { //Enquanto os dois garfos nÃ£o estiverem disponiveis, execute o loop
        pthread_cond_wait(&cond, &mutex);   //Espera sinal original da funcao put_forks
    }
    forks[(id+1)%5] = 1;    //Quando os garfos estiverem disponiveis, pegue-os
    forks[id] = 1;
    printf("                                O filosofo %d pegou os garfos\n", id);
    pthread_mutex_unlock(&mutex); //Sai da regiao critica
}

void put_forks(int id){
    pthread_mutex_lock(&mutex); //Acessa a regiao critica
    forks[id] = 0;  //Devolve os garfos
    forks[(id+1)%5] = 0;
    printf("O filosofo %d soltou os garfos\n", id);
    pthread_mutex_unlock(&mutex);   //Sai da regiao critica
    pthread_cond_broadcast(&cond);  //Manda sinal para todos
}

void think(int id){
    sleep(pensar); //"dorme" durante pensar segundos
}

void eat(int id){
    sleep(comer); //"dorme" durante comer segundos
}
