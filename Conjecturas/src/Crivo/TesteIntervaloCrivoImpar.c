#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

void testeCrivo(long long N) {

    if (N < 2) {
        printf("Nao ha primos.\n");
        return;
    }

    long long tamanho = (N / 2) + 1; // apenas impares
    bool *composto = calloc(tamanho, sizeof(bool));

    if (!composto) {
        printf("Erro de alocacao.\n");
        return;
    }

    long long total_primos = 1; // contando o 2
    long long total_marcacoes = 0;

    clock_t inicio = clock();

    long long limite = sqrt(N);

    for (long long i = 1; (2*i + 1) <= limite; i++) {

        if (!composto[i]) {

            long long p = 2*i + 1;
            long long inicio_j = (p*p - 1) / 2;

            for (long long j = inicio_j; j < tamanho; j += p) {
                if (!composto[j]) {
                    composto[j] = true;
                    total_marcacoes++;
                }
            }
        }
    }

    for (long long i = 1; i < tamanho; i++)
        if (!composto[i])
            total_primos++;

    clock_t fim = clock();
    double tempo = (double)(fim - inicio) / CLOCKS_PER_SEC;
    double throughput = (tempo > 0) ? N / tempo : 0;

    printf("\n===== RESULTADOS (BOOL) =====\n");
    printf("N = %lld\n", N);
    printf("Primos encontrados = %lld\n", total_primos);
    printf("Total de marcacoes = %lld\n", total_marcacoes);
    printf("Memoria usada = %lld bytes\n", tamanho * sizeof(bool));
    printf("Tempo total = %.4f s\n", tempo);
    printf("Throughput = %.0f numeros/s\n", throughput);

    free(composto);
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Uso: %s <N>\n", argv[0]);
        return 1;
    }

    long long N = atoll(argv[1]);
    testeCrivo(N);

    return 0;
}