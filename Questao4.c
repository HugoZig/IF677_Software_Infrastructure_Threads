#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#define nodes 5

typedef struct{
    int begin, end;
    long long int value;
} tupla;                    //== "edges"

typedef struct{
    long long int** matrix;
    int numedge, nvert;
} graph;


int leader[nodes];          //lider de cada vertice
bool use[nodes];            //indica se um vertice e usado como lider ou nao
tupla ledges[nodes];        //arestas a serem encontradas pelas threads
graph g;
graph mst;

graph createg(int n){       //iniciar grafo
    int i, j;
    graph g;
    g.matrix = (long long int**) malloc(sizeof(long long int*) * n);
    if(g.matrix == NULL){
        printf("Erro de alocacao...");
        exit(1);
    }
    for(i=0; i<n; i++){
        g.matrix[i] = (long long int*) malloc(sizeof(long long int) * n);
        if(g.matrix[i] == NULL){
            printf("Erro de alocacao...");
            exit(1);
        }
    }
    for(i=0; i<n; i++)
        for(j=0; j<n; j++)
            g.matrix[i][j] = - 1;       //arestas inexistentes em -1
    g.numedge = 0;
    g.nvert = n;
    return g;
}

int first(graph* g, int v){         //achar primeira aresta pela matriz de adjacencia
    int i;
    for(i=0;i<g->nvert;i++)
        if(g->matrix[v][i] != -1) return i;
    return g->nvert;
}

int next(graph* g, int v, int w){   //segunda aresta em diante
    int i;
    for(i=w+1;i<g->nvert;i++)
        if(g->matrix[v][i] != -1) return i;
    return g->nvert;
}

void setedge(graph *g, int i, int j, long long int wt){ //inserir aresta
    if(wt != -1){
        if(g->matrix[i][j] == -1) g->numedge++;
        g->matrix[i][j] = wt;
        g->matrix[j][i] = wt;
    }
}

void replace(int i, int j){     //buscar os vertices com lider j e sibstituir por i
    int k;
    for(k=0; k<g.nvert; k++)
        if(leader[k] == j)
            leader[k] = i;
}

void atualizar(int i){ //nao ha mais vertices com lider de indice i que sejam lideres
    int k;
    for(k=0; k<g.nvert; k++)
        if(leader[k] == i)
            use[k] = 0;
}

void uniao(int i, int j){   //atualizar lideres
    if(leader[i] > leader[j]){
        atualizar(leader[i]);
        replace(leader[j], leader[i]);
    }
    else{
        atualizar(leader[j]);
        replace(leader[i], leader[j]);
    }
}

int find(int i){   //achar o respresentante do componente de cada aresta
    return leader[i];
}

void reiniciararestas(void){  //para arestas de loops anteriores nao interferirem
    int i;
    for(i=0; i<g.nvert; i++){
        ledges[i].value = -1;
        ledges[i].begin = 0;
        ledges[i].end = 0;
    }
}

int nextnode(int i){ //identificar proximo vertice na componente
    int k;
    for(k=i+1; k<nodes; k++)
        if(leader[k] == leader[i])
            return k;
    return g.nvert;
}

void* find_edges(void* arg){   //cada thread acha a aresta de sua componente
    int par = (*(int*) arg);
    int w, i;
    tupla candidate;
    i = par;
    while(i<g.nvert){        //i é o vértice cujas arestas serao analisadas
        w = first(&g, i);    //w contem as arestas a serem analisadas
        while(w<g.nvert){
            candidate.begin = i;
            candidate.end = w;
            candidate.value = g.matrix[i][w];
            if((candidate.value < ledges[leader[i]].value || ledges[leader[i]].value == -1) && leader[i] != leader[w]){
    //acontece se find diferente, e peso menor que a menor aresta ate agora ou nao foi achada nenhum aresta ainda
                ledges[leader[i]] = candidate;
            }
            w = next(&g, i, w);
        }
        i = nextnode(i);
    }
    if(ledges[par].value != -1){
        printf("thread %d achou uma aresta de peso %lld\n", par, ledges[par].value);
    }
    else{
        printf("Thread %d nao achou aresta com outra componente\n", par);
    }
    if(arg != NULL) free(arg);
    pthread_exit(NULL);
}

void boruvka(void){
    int i, naoconexo;
    int arestas_restantes = g.nvert - 1;
    
    pthread_t threads[nodes];
    int *parametro[nodes];
    
    mst = createg(nodes);
    
    for(i=0;i<g.nvert;i++){
        leader[i] = i;
        use[i] = 1;
    }
    
    while(arestas_restantes){  //enquanto tiver arestas a serem adicionadas na MST
        printf("%d arestas a serem encontradas\n", arestas_restantes);
        naoconexo = arestas_restantes;
        reiniciararestas();
        for(i=0;i<g.nvert;i++){
                if(use[i]){
                parametro[i] = (int*) malloc (sizeof(int));
                if(parametro[i] == NULL){
                    printf("Erro de alocacao...");
                    exit(1);
                }
                *parametro[i] = i;
                if(pthread_create(&threads[i],NULL, find_edges, (void *)parametro[i])){
                    printf("Erro de criacao...");
                    exit(2);
                }
            }
        }
        
        for(i=0; i< g.nvert; i++){
            if(use[i])
                if(pthread_join(threads[i], NULL)){
                    printf("Erro de join...");
                    exit(3);
                }
        }
        
        for(i=0;i<g.nvert;i++){
            if(ledges[i].value != -1)     //-1 == nao inicializada
                printf("aresta de peso %lld foi achada\n", ledges[i].value);
            if(find(ledges[i].begin) != find(ledges[i].end)){
                uniao(ledges[i].begin, ledges[i].end);
                setedge(&mst, ledges[i].begin, ledges[i].end, ledges[i].value);
                arestas_restantes--;
                printf("%d %d %lld\n", ledges[i].begin, ledges[i].end, ledges[i].value);
            }
        }
        if(naoconexo == arestas_restantes){ //se nao foi feita nenhuma uniao
            printf("Erro... Grafo nao conexo\n");
            exit(1);
        }
    }
}

int main()
{
    int arcs, begin, end, i;
    long long int wei;
    g = createg(nodes);
    /*printf("Digite a quantidade de arestas:\n");    //leitura por scan
    scanf("%d", &arcs);
        while(arcs){
            arcs--;
            scanf("%d %d %lld", &begin, &end, &wei);
            setedge(&g, begin, end, wei);
        }*/
    
    //inserir as chamadas de setedge aqui caso não usar o scan
    setedge(&g, 0, 1, 10);
    setedge(&g, 0, 2, 3);
    setedge(&g, 0, 3, 20);
    setedge(&g, 3, 1, 5);
    setedge(&g, 2, 1, 2);
    setedge(&g, 3, 4, 11);
    setedge(&g, 2, 4, 15);
    
    boruvka();
    for(i=0;i<nodes;i++){
        if(g.matrix[i] != NULL) free(g.matrix[i]);
        if(mst.matrix[i] != NULL) free(mst.matrix[i]);
    }
    if(g.matrix != NULL) free(g.matrix);
    if(mst.matrix != NULL) free(mst.matrix);
    return 0;
}
