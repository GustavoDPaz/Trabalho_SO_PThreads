#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define MATRIZ_LINHAS 1000
#define MATRIZ_COLUNAS 1000
#define BLOCO_LINHAS 100
#define BLOCO_COLUNAS 100
#define NUM_THREADS 4
#define SEMENTE 42

int **matriz = NULL;
int total_primos_paralelo = 0;
int proximo_macrobloco = 0;
int total_macroblocos = 0;

pthread_mutex_t mutex_bloco;
pthread_mutex_t mutex_primos;

int ehPrimo(int n);
void alocarMatriz();
void preencherMatriz();
void liberarMatriz();
int buscaSerial();
void *funcaoThread(void *arg);
void buscaParalela();

/*
int proxIndex(); // acho que a gente vai ter que criar uma função pra descobrir
// qual vai ser o proximo indice tlgd? Não sei explicar direito, mas como a gente vai
// ter que percorrer linha e coluna né, não da pra ficar fazendo for só com i++ o tempo
// inteiro. sei lá mano, to só declarando aqui já pra poder botar de placeholder no cod.
// Na verdade ao invez de proximo, talvez seja uma função que retorna uma array com 2 de
// tamanho com a linha e a coluna; muitas perguntas.

int percorrer(int lin, int col, int m[lin][col])
{
    // [[1, 2], [1, 3], [1,4]]
    int n = 0;
    int tamanho = lin * col;
    int l = 0, c = 0;
    while (n < tamanho)
    {
        primo(m[l, c]);
        l++;
    }
    return 0;
}
*/

int main()
{
    if (MATRIZ_LINHAS % BLOCO_LINHAS != 0 || MATRIZ_COLUNAS % BLOCO_COLUNAS != 0)
    {
        printf("Erro: As dimensoes da matriz devem ser multiplas do macrobloco!\n");
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

    clock_t inicio_serial = clock();
    int primos_serial = buscaSerial();
    clock_t fim_serial = clock();
    double tempo_serial = (double)(fim_serial - inicio_serial) / CLOCKS_PER_SEC;

    printf("Serial - Primos: %d | Tempo: %.4f s\n", primos_serial, tempo_serial);

    clock_t inicio_paralelo = clock();
    buscaParalela();
    clock_t fim_paralelo = clock();
    double tempo_paralelo = (double)(fim_paralelo - inicio_paralelo) / CLOCKS_PER_SEC;

    printf("Paralelo - Primos: %d | Tempo: %.4f s\n", total_primos_paralelo, tempo_paralelo);

    if (tempo_paralelo > 0.0)
    {
        double speedup = tempo_serial / tempo_paralelo;
        printf("Speedup: %.2fx\n", speedup);
    }

    liberarMatriz();

    return 0;
}

int ehPrimo(int n)
{
    if (n < 2)
    {
        return 0;
    }

    int squirtle = sqrt(n);
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
    int contador = 0;
    for (int i = 0; i < MATRIZ_LINHAS; i++)
    {
        for (int j = 0; j < MATRIZ_COLUNAS; j++)
        {
            if (ehPrimo(matriz[i][j]))
            {
                contador++;
            }
        }
    }
    return contador;
}

void *funcaoThread(void *arg)
{
    int id = *(int *)arg;
    int blocos_por_linha = MATRIZ_COLUNAS / BLOCO_COLUNAS;
    int blocos_processados = 0;

    while (1)
    {
        int meu_bloco = -1;

        pthread_mutex_lock(&mutex_bloco);
        if (proximo_macrobloco < total_macroblocos)
        {
            meu_bloco = proximo_macrobloco;
            proximo_macrobloco++;
        }
        pthread_mutex_unlock(&mutex_bloco);

        if (meu_bloco == -1)
        {
            break;
        }

        blocos_processados++;

        int bloco_lin = meu_bloco / blocos_por_linha;
        int bloco_col = meu_bloco % blocos_por_linha;

        int lin_inicio = bloco_lin * BLOCO_LINHAS;
        int lin_fim = lin_inicio + BLOCO_LINHAS;

        int col_inicio = bloco_col * BLOCO_COLUNAS;
        int col_fim = col_inicio + BLOCO_COLUNAS;

        int primos_deste_bloco = 0;
        for (int i = lin_inicio; i < lin_fim; i++)
        {
            for (int j = col_inicio; j < col_fim; j++)
            {
                if (ehPrimo(matriz[i][j]))
                {
                    primos_deste_bloco++;
                }
            }
        }

        pthread_mutex_lock(&mutex_primos);
        total_primos_paralelo += primos_deste_bloco;
        pthread_mutex_unlock(&mutex_primos);
    }

    return NULL;
}

void buscaParalela()
{
    total_primos_paralelo = 0;
    proximo_macrobloco = 0;

    pthread_mutex_init(&mutex_bloco, NULL);
    pthread_mutex_init(&mutex_primos, NULL);

    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++)
    {
        ids[i] = i;
        pthread_create(&threads[i], NULL, funcaoThread, &ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex_bloco);
    pthread_mutex_destroy(&mutex_primos);
}
