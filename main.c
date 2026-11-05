#include <pthread.h>
#include <stdio.h>

int primo(int n);
int percorrer(int lin, int col, int m[lin][col]);
int proxIndex(); /* acho que a gente vai ter que criar uma função pra descobrir
qual vai ser o proximo indice tlgd? Não sei explicar direito, mas como a gente vai
ter que percorrer linha e coluna né, não da pra ficar fazendo for só com i++ o tempo
inteiro. sei lá mano, to só declarando aqui já pra poder botar de placeholder no cod.
Na verdade ao invez de proximo, talvez seja uma função que retorna uma array com 2 de
tamanho com a linha e a coluna; muitas perguntas.
*/

#define MATRIZ_LINHAS = 3;
#define MATRIZ_COLUNAS = 3;
#define BLOCO_LINHAS = 2;
#define BLOCO_COLUNAS = 2;

int main()
{
    int *ptr;

    return 0;
}

int primo(int n)
{
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