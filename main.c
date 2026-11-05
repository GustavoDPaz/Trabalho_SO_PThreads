#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// ============================================================================
// CONFIGURACOES PARAMETRIZADAS (conforme pedido no PDF)
// ============================================================================
// Dimensoes da matriz (o professor testara com tamanhos grandes como 10000x10000,
// aqui deixamos um tamanho bom para testar sem travar o computador)
#define MATRIZ_LINHAS 1000
#define MATRIZ_COLUNAS 1000

// Tamanho dos macroblocos (submatrizes)
#define BLOCO_LINHAS 100
#define BLOCO_COLUNAS 100

// Quantidade de threads trabalhadoras
#define NUM_THREADS 4

// Semente fixa para que os numeros aleatorios sejam sempre os mesmos em todos os testes
#define SEMENTE 42

// ============================================================================
// VARIAVEIS GLOBAIS (compartilhadas entre as threads)
// ============================================================================
int **matriz = NULL;                // Matriz principal alocada dinamicamente
int total_primos_paralelo = 0;      // Contador global de primos achados pelas threads
int proximo_macrobloco = 0;         // Indice do proximo bloco a ser pego por uma thread
int total_macroblocos = 0;          // Quantidade total de macroblocos na matriz

// Mutexes para proteger as secoes criticas
pthread_mutex_t mutex_bloco;        // Protege a variavel 'proximo_macrobloco'
pthread_mutex_t mutex_primos;       // Protege a variavel 'total_primos_paralelo'

// ============================================================================
// PROTOTIPOS DAS FUNCOES
// ============================================================================
int ehPrimo(int n);
int primo(int n);
void alocarMatriz();
void preencherMatriz();
void liberarMatriz();
int buscaSerial();
void *funcaoThread(void *arg);
void buscaParalela();

/* ============================================================================
   CODIGO FEITO A MAO ANTERIORMENTE (GUARDADO CASO QUEIRA VOLTAR)
   ============================================================================

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

int percorrerSerialAlternativo(int lin, int col, int **m)
{
    int contadorPrimos = 0;
    for (int i = 0; i < lin; i++)
    {
        for (int j = 0; j < col; j++)
        {
            if (ehPrimo(m[i][j]))
            {
                contadorPrimos++;
            }
        }
    }
    return contadorPrimos;
}
============================================================================ */

// ============================================================================
// FUNCAO PRINCIPAL
// ============================================================================
int main()
{
    printf("====================================================\n");
    printf("   TRABALHO DE SISTEMAS OPERACIONAIS - PTHREADS    \n");
    printf("====================================================\n");
    printf("Matriz: %d x %d\n", MATRIZ_LINHAS, MATRIZ_COLUNAS);
    printf("Macrobloco: %d x %d\n", BLOCO_LINHAS, BLOCO_COLUNAS);
    printf("Numero de Threads: %d\n", NUM_THREADS);

    // Calcula quantos blocos existem no total
    int blocos_linhas = MATRIZ_LINHAS / BLOCO_LINHAS;
    int blocos_colunas = MATRIZ_COLUNAS / BLOCO_COLUNAS;
    total_macroblocos = blocos_linhas * blocos_colunas;
    printf("Total de Macroblocos: %d\n\n", total_macroblocos);

    // 1. Aloca e preenche a matriz
    alocarMatriz();
    preencherMatriz();

    // 2. Executa a Busca Serial
    printf("Iniciando Busca Serial...\n");
    clock_t inicio_serial = clock();
    int primos_serial = buscaSerial();
    clock_t fim_serial = clock();
    double tempo_serial = (double)(fim_serial - inicio_serial) / CLOCKS_PER_SEC;

    printf("-> Primos encontrados (Serial): %d\n", primos_serial);
    printf("-> Tempo decorrido (Serial): %.4f segundos\n\n", tempo_serial);

    // 3. Executa a Busca Paralela (com PThreads)
    printf("Iniciando Busca Paralela (%d threads)...\n", NUM_THREADS);
    clock_t inicio_paralelo = clock();
    buscaParalela();
    clock_t fim_paralelo = clock();
    double tempo_paralelo = (double)(fim_paralelo - inicio_paralelo) / CLOCKS_PER_SEC;

    printf("-> Primos encontrados (Paralelo): %d\n", total_primos_paralelo);
    printf("-> Tempo decorrido (Paralelo): %.4f segundos\n\n", tempo_paralelo);

    // 4. Calculo do Speedup (Amdahl)
    if (tempo_paralelo > 0.0)
    {
        double speedup = tempo_serial / tempo_paralelo;
        printf("====================================================\n");
        printf("SPEEDUP ALCANCADO: %.2fx mais rapido\n", speedup);
        printf("====================================================\n");
    }

    // Verificacao de corretude
    if (primos_serial == total_primos_paralelo)
    {
        printf("[OK] A contagem das threads bateu perfeitamente com a serial!\n");
    }
    else
    {
        printf("[ERRO] Os totais de primos estao diferentes! Condicao de corrida?\n");
    }

    // 5. Libera a memoria antes de sair
    liberarMatriz();

    return 0;
}

// ============================================================================
// VERIFICACAO DE PRIMOS (Mantendo o raciocinio e a variavel 'squirtle')
// ============================================================================
int primo(int n)
{
    // 0 e 1 nao sao primos (matriz gera de 0 a 31999)
    if (n < 2)
    {
        return 0;
    }

    int squirtle = sqrt(n);
    for (int i = 2; i <= squirtle; i++)
    {
        if (n % i == 0)
        {
            return 0; // achou divisor, nao e primo
        }
    }
    return 1; // e primo
}

// O professor pediu na especificacao a funcao com nome ehPrimo
int ehPrimo(int n)
{
    return primo(n);
}

// ============================================================================
// ALOCACAO DINAMICA DA MATRIZ (Estilo simples de apostila de C)
// ============================================================================
void alocarMatriz()
{
    matriz = (int **)malloc(MATRIZ_LINHAS * sizeof(int *));
    if (matriz == NULL)
    {
        printf("Erro ao alocar memoria para as linhas da matriz!\n");
        exit(1);
    }

    for (int i = 0; i < MATRIZ_LINHAS; i++)
    {
        matriz[i] = (int *)malloc(MATRIZ_COLUNAS * sizeof(int));
        if (matriz[i] == NULL)
        {
            printf("Erro ao alocar memoria para as colunas da matriz na linha %d!\n", i);
            exit(1);
        }
    }
}

// ============================================================================
// PREENCHIMENTO DA MATRIZ COM NUMEROS ALEATORIOS (0 a 31999)
// ============================================================================
void preencherMatriz()
{
    // Semente fixa para que todos os testes gerem a mesma matriz
    srand(SEMENTE);

    for (int i = 0; i < MATRIZ_LINHAS; i++)
    {
        for (int j = 0; j < MATRIZ_COLUNAS; j++)
        {
            matriz[i][j] = rand() % 32000;
        }
    }
}

// ============================================================================
// LIBERACAO DA MEMORIA (free linha por linha, depois o vetor de ponteiros)
// ============================================================================
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

// ============================================================================
// BUSCA SERIAL (Um por um, tempo de referencia)
// ============================================================================
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

// ============================================================================
// FUNCAO EXECUTADA POR CADA THREAD (Worker)
// ============================================================================
void *funcaoThread(void *arg)
{
    (void)arg; // Apenas para evitar aviso de variavel nao usada

    int blocos_por_linha = MATRIZ_COLUNAS / BLOCO_COLUNAS;

    while (1)
    {
        int meu_bloco = -1;

        // ---------------------------------------------------------
        // SECAO CRITICA 1: Pegar o proximo macrobloco disponivel
        // ---------------------------------------------------------
        pthread_mutex_lock(&mutex_bloco);
        if (proximo_macrobloco < total_macroblocos)
        {
            meu_bloco = proximo_macrobloco;
            proximo_macrobloco++;
        }
        pthread_mutex_unlock(&mutex_bloco);

        // Se nao ha mais blocos livres para processar, a thread encerra
        if (meu_bloco == -1)
        {
            break;
        }

        // ---------------------------------------------------------
        // CALCULO DAS COORDENADAS DO BLOCO (Sem guardar vetores na RAM)
        // ---------------------------------------------------------
        int bloco_lin = meu_bloco / blocos_por_linha;
        int bloco_col = meu_bloco % blocos_por_linha;

        int lin_inicio = bloco_lin * BLOCO_LINHAS;
        int lin_fim = lin_inicio + BLOCO_LINHAS;

        int col_inicio = bloco_col * BLOCO_COLUNAS;
        int col_fim = col_inicio + BLOCO_COLUNAS;

        // ---------------------------------------------------------
        // PROCESSAMENTO DO BLOCO: Conta primos em variavel local
        // ---------------------------------------------------------
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

        // ---------------------------------------------------------
        // SECAO CRITICA 2: Somar o resultado local no total global
        // ---------------------------------------------------------
        pthread_mutex_lock(&mutex_primos);
        total_primos_paralelo += primos_deste_bloco;
        pthread_mutex_unlock(&mutex_primos);
    }

    return NULL;
}

// ============================================================================
// BUSCA PARALELA: Cria as threads, espera todas terminarem e junta os resultados
// ============================================================================
void buscaParalela()
{
    // Reseta as variaveis globais antes de comecar a busca
    total_primos_paralelo = 0;
    proximo_macrobloco = 0;

    // Inicializa os mutexes
    pthread_mutex_init(&mutex_bloco, NULL);
    pthread_mutex_init(&mutex_primos, NULL);

    pthread_t threads[NUM_THREADS];

    // Cria as N threads
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_create(&threads[i], NULL, funcaoThread, NULL);
    }

    // Espera todas as threads terminarem (pthread_join)
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // Destroi os mutexes apos o uso
    pthread_mutex_destroy(&mutex_bloco);
    pthread_mutex_destroy(&mutex_primos);
}
