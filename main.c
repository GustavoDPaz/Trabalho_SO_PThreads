// apaga os comentario mais doidinho antes da gente enviar

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MATRIZ_LINHAS 10000
#define MATRIZ_COLUNAS 10000
#define BLOCO_LINHAS 1
#define BLOCO_COLUNAS 1
#define NUM_THREADS 4 // eu to pensando aqui, a especificação falou pra só considerar os núcleos de performance, mas ficou meio
// aberto se isso quer dizer que tem que forçar o programa a utilizar só os nucleos de performance (tipo aquela opção do gerenciador)
// de tarefas de limitar quais CPUs o processo pode usar) ou se é só pra colocar o numero de threads equivalente mesmo.
// (Eu, André, tenho um notebook com um i5 12450HX, que tem 4 núcleos de performance e 4 de eficiência / 12 threads)
#define SEMENTE 42

int **matriz = NULL;
int total_primos_paralelo = 0;
int proximo_bloco = 0;
int total_macroblocos = 0;

pthread_mutex_t mutex_bloco;  // mutex q controla o acesso a blocos
pthread_mutex_t mutex_primos; // mutex q controla acesso ao contador de primos

int ehPrimo(int n);
void alocarMatriz();
void preencherMatriz();
void liberarMatriz();
int buscaSerial();
void *funcaoThread(void *arg);
void buscaParalela();

int main()
{
    // A especificação não dizia o que fazer caso o tamanho do bloco seja divisivel pelo tamanho da matriz mas o numero de linhas
    // ou colunas do bloco não é divisível pelo numero de linhas ou colunas da matriz, então decidimos simplesmente não deixar.
    // Eu acho que eu fui a unica pessoa que ficou quebrando a cabeça com isso, então acho que era isso que o trabalho queria
    // mesmo, mas acho que seria interessante revisitar esse problema depois de divisão de matriz sem necessariamente exigir
    // blocos "visualmente continuos".
    if (MATRIZ_LINHAS % BLOCO_LINHAS != 0 || MATRIZ_COLUNAS % BLOCO_COLUNAS != 0)
    {
        printf("ATENCAO!!!!!!! Os numeros colocados para o tamanho dos blocos (linhas/colunas) nao sao divisiveis pelas linhas/colunas da matriz.");
        return 1;
    }

    printf("Matriz: %d x %d\n", MATRIZ_LINHAS, MATRIZ_COLUNAS);
    printf("Macrobloco: %d x %d\n", BLOCO_LINHAS, BLOCO_COLUNAS);
    printf("Numero de Threads: %d\n", NUM_THREADS);

    int blocos_linhas = MATRIZ_LINHAS / BLOCO_LINHAS;
    int blocos_colunas = MATRIZ_COLUNAS / BLOCO_COLUNAS;
    total_macroblocos = blocos_linhas * blocos_colunas;
    printf("Total de Macroblocos: %d\n\n", total_macroblocos);

    alocarMatriz();
    preencherMatriz();
    clock_t ini_serial = clock();
    int primos_serial = buscaSerial();
    clock_t fim_serial = clock();
    double t_serial = (double)(fim_serial - ini_serial) / CLOCKS_PER_SEC;
    // todo mundo usa os mesmo nomes de variável, o que é muito chato, mas acho que eu nao tenho capacidade mental de usar o tipo
    // de nome que eu usava em prog 2 num problema complicado desses então estou deixando aqui nomes bonitinhos

    printf("Serial - Primos: %d | Tempo: %.4f s\n", primos_serial, t_serial);

    clock_t ini_paralelo = clock();
    buscaParalela();
    clock_t fim_paralelo = clock();
    double t_paralelo = (double)(fim_paralelo - ini_paralelo) / CLOCKS_PER_SEC;

    printf("Paralelo - Primos: %d | Tempo: %.4f s\n", total_primos_paralelo, t_paralelo);

    if (t_paralelo > 0.0)
    {
        double speedup = t_serial / t_paralelo;
        printf("Speedup: %.2fx\n", speedup);
    }

    liberarMatriz();

    return 0;
}

int ehPrimo(int n)
{
    // sinceramente, não sei se deixar isso aqui deixa a função "menos otimizada" que nem a especificação pede
    // ou mais otimizada, como os testes variam muito acho que precisa de uma análise estatística que eu n vou
    // fazer nem ferrando pq n pediram mesmo
    if (n < 2)
    {
        return 0;
    }

    int squirtle = sqrt(n);
    // probleminha de prog 1, saudades
    for (int i = 2; i <= squirtle; i++)
    {
        if (n % i == 0)
        {
            return 0;
        }
    }
    return 1;
}

void alocarMatriz()
{
    matriz = (int **)calloc(MATRIZ_LINHAS, sizeof(int *));
    if (matriz == NULL)
    {
        exit(1);
    }

    for (int i = 0; i < MATRIZ_LINHAS; i++)
    {
        matriz[i] = (int *)calloc(MATRIZ_COLUNAS, sizeof(int));
        if (matriz[i] == NULL)
        {
            exit(1);
        }
    }
}

void preencherMatriz()
{
    srand(SEMENTE);

    for (int i = 0; i < MATRIZ_LINHAS; i++)
    {
        for (int j = 0; j < MATRIZ_COLUNAS; j++)
        {
            matriz[i][j] = rand() % 32000;
        }
    }
}

void liberarMatriz()
{
    if (matriz != NULL)
    {
        for (int i = 0; i < MATRIZ_LINHAS; i++)
        {
            free(matriz[i]);
        }
        free(matriz);
        matriz = NULL;
    }
}

int buscaSerial()
{
    // c = contador 😱
    int c = 0;
    for (int i = 0; i < MATRIZ_LINHAS; i++)
    {
        for (int j = 0; j < MATRIZ_COLUNAS; j++)
        {
            if (ehPrimo(matriz[i][j]))
            {
                c++;
            }
        }
    }
    return c;
}

void *funcaoThread(void *arg)
{
    (void)arg;

    int blocos_por_linha = MATRIZ_COLUNAS / BLOCO_COLUNAS;

    while (1)
    {
        int bloco_atual = -1;

        pthread_mutex_lock(&mutex_bloco);
        if (proximo_bloco < total_macroblocos)
        {
            bloco_atual = proximo_bloco;
            proximo_bloco++;
        }
        pthread_mutex_unlock(&mutex_bloco);

        if (bloco_atual == -1)
        {
            // finalmente uma matéria nesse curso que a gente pode usar while "true" e break
            break;
        }

        int bloco_lin = bloco_atual / blocos_por_linha;
        int bloco_col = bloco_atual % blocos_por_linha;

        int lin_inicio = bloco_lin * BLOCO_LINHAS;
        int lin_fim = lin_inicio + BLOCO_LINHAS;

        int col_inicio = bloco_col * BLOCO_COLUNAS;
        int col_fim = col_inicio + BLOCO_COLUNAS;

        int primos_bloco = 0;
        for (int i = lin_inicio; i < lin_fim; i++)
        {
            for (int j = col_inicio; j < col_fim; j++)
            {
                if (ehPrimo(matriz[i][j]))
                {
                    primos_bloco++;
                }
            }
        }

        pthread_mutex_lock(&mutex_primos);
        total_primos_paralelo += primos_bloco;
        pthread_mutex_unlock(&mutex_primos);
    }

    return NULL;
}

void buscaParalela()
{
    total_primos_paralelo = 0;
    proximo_bloco = 0;

    pthread_mutex_init(&mutex_bloco, NULL);
    pthread_mutex_init(&mutex_primos, NULL);

    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&threads[i], NULL, funcaoThread, NULL);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex_bloco);
    pthread_mutex_destroy(&mutex_primos);
}
