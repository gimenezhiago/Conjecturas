#include <stdio.h>
#include <stdbool.h> //Para usar booleano
#include <stdlib.h> //Para usar atoll
#include <omp.h> //Para paralelizar com OpenMP
#include <time.h> //Para medir tempo
#include <math.h>

bool ehPrimo(long long n) {
    if (n <= 1) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    for (long long i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

inline bool ehQuadradoPrimo(long long n) {
    long long raiz = (long long)sqrt(n);
    return (raiz * raiz == n && ehPrimo(raiz));
}

long long proximoPrimo(long long n) {
    long long p = n + 1;
    while (!ehPrimo(p)) {
        p++;
    }
    return p;
}

inline long long aplicarRegrasTri(long long n) {
    if (n <= 1) return n;
    long long divisor = 2;
    while (divisor <= n && n % divisor != 0) {
        divisor = proximoPrimo(divisor);
    }
    if (divisor > n) return n;
    return n / divisor;
}

inline long long triangular(int m) {
    return (long long)m * (m + 1) / 2;
}

void testeIntervalo(long long fim) {
    const long long LIMITE = 1000000000000; //1 trilhão
    volatile int abortFlag = 0; // sinaliza para encerrar o laço quando um caso falhar
    long long i;
    
    double inicio = omp_get_wtime();

    #pragma omp parallel for schedule(dynamic, 1000) default(none) shared(fim, LIMITE, abortFlag) private(i)
    for (i = 1; i <= fim; i++) {
        int localAbort = 0;
        #pragma omp atomic read
        localAbort = abortFlag;
        if (localAbort) continue; // outro thread já pediu abort

        long long atual = triangular(i) * triangular(i + 1);
        long long contador = 0;
        bool convergiu = false;

        while (contador < LIMITE) {
            #pragma omp atomic read
            localAbort = abortFlag;
            if (localAbort) break;

            if (ehPrimo(atual) || ehQuadradoPrimo(atual)) {
                convergiu = true;
                break;
            }

            atual = aplicarRegrasTri(atual);
            contador++;

            if (atual <= 0) { // overflow / valor inválido
                #pragma omp critical
                {
                    printf("O numero %lld gerou valor invalido/overflow em %lld iteracoes. ultimo valor = %lld\n", i, contador, atual);
                }
                #pragma omp atomic write
                abortFlag = 1;
                break;
            }
        }

        if (!convergiu) {
            #pragma omp critical
            {
                printf("O numero %lld nao convergiu (>%lld iteracoes). ultimo valor = %lld\n", i, LIMITE, atual);
            }
            #pragma omp atomic write
            abortFlag = 1;
        }

        #pragma omp critical
        {
            if (i % 100000 == 0) {
                printf("Testados %lld numeros\n", i);
            }
        }
    }

    double fim_tempo = omp_get_wtime();
    double tempo_decorrido = fim_tempo - inicio;
    double throughput = (tempo_decorrido > 0) ? fim / tempo_decorrido : 0;
    
    printf("\nTempo total: %.2f segundos\n", tempo_decorrido);
    printf("Throughput: %.0f numeros/segundo\n", throughput);
    printf("Teste concluido ate %lld\n", fim);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <fim> [num_threads]\n", argv[0]);
        return 1;
    }

    long long fim = atoll(argv[1]);
    if (fim <= 0) {
        printf("Parametro invalido: %s\n", argv[1]);
        return 1;
    }

    if (argc >= 3) {
        int t = atoi(argv[2]);
        if (t > 0) {
            omp_set_num_threads(t);
            printf("Usando %d threads OpenMP\n", t);
        }
    }

    printf("Testando ate %lld\n", fim);
    testeIntervalo(fim);

    return 0;
}

// Para compilar com GCC (MinGW/MSYS): gcc -O3 -fopenmp TesteIntervaloTriParalelo.c -o TesteIntervaloTriParalelo
// Para rodar: ./TesteIntervaloTriParalelo 100
