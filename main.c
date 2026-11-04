#include <pthread.h>
#include <stdio.h>

int main()
{
    int *ptr;

    return 0;
}

int primo(int n)
{
    for (int i = 2; i <= sqrt(n); i++)
    {
        if (n % i == 0)
        {
            return 0;
        }
    }
    return 1;
}

int serial(int lin, int col, int m[lin][col])
{
}