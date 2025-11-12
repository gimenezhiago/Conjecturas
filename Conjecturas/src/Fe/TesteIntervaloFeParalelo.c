#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <omp.h>

long long somaDigitos(long long n) {
    long long soma = 0;
    for (; n > 0; n /= 10) {
        soma += n % 10;
    }
    return soma;
}

long long aplicarRegras(long long n) {
    if (n % 2 == 0) {
        return n / 2;
    }
    if (n % 3 == 0) {
        return n / 3;
    }
    if (n % 5 == 0) {
        return n / 5;
    }
    if (n % 7 == 0) {
        return n / 7;
    }

    return n + somaDigitos(n);
}

void testeIntervalo (long long fim) {
    const long long LIMITE = 1000000000000;
    #pragma omp parallel for schedule(dynamic) // Paraleliza o for com OpenMP
    for (long long i = 1; i <= fim; i++) {
        long long atual = i;
        long long contador = 0;
        bool convergiu = false;

        while (contador < LIMITE) {
            if (atual == 1) {
                convergiu = true;
                break;
            }

            atual = aplicarRegras(atual);
            contador++;

            if (atual <= 0) {
                printf("O numero %lld gerou valor invalido em %lld iteracoes. ultimo valor = %lld\n", i, contador, atual);
                break;
            }
        }

        if (!convergiu) {
            printf("O numero %lld nao convergiu (>%lld iteracoes). ultimo valor = %lld\n", i, LIMITE, atual);
            break;
        }
        
        #pragma omp critical //seção crítica para evitar que múltiplas threads imprimam ao mesmo tempo
        if (i % 100000 == 0) {
            printf("Testados %lld numeros\n", i);
        }
    }
}

int main (int argc, char *argv[]) {
     if (argc < 2) {
        printf("Uso: %s <fim>\n", argv[0]);
        return 1;
    }

    long long fim = atoll(argv[1]);
    if (fim <= 0) {
        printf("Parametro invalido: %s\n", argv[1]);
        return 1;
    }

    printf("Testando ate %lld\n", fim);
    testeIntervalo(fim);

    return 0;
}


// Para compilar: gcc -O3 TesteIntervaloFeParalelo.c -o TesteIntervaloFeParalelo -fopenmp
// Para rodar: ./TesteIntervaloFeParalelo 100