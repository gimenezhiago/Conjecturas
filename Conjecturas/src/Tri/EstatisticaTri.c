#include <stdio.h>
#include <stdbool.h> //Para usar booleano
#include <stdlib.h> //Para usar atoll
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

bool ehQuadradoPrimo(long long n) {
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

long long aplicarRegrasTri(long long n) {
    if (n <= 1) return n;
    long long divisor = 2;
    while (divisor <= n && n % divisor != 0) {
        divisor = proximoPrimo(divisor);
    }
    if (divisor > n) return n;
    return n / divisor;
}

long long triangular(int m) {
    return (long long)m * (m + 1) / 2;
}

long long totalPassos = 0;
long long totalNumeros = 0;

void analisar(long long n) {
    long long atual = triangular(n) * triangular(n + 1);
    long long passos = 0;

    while (!(ehPrimo(atual) || ehQuadradoPrimo(atual))) {
        atual = aplicarRegrasTri(atual);
        passos++;
        if (passos > 1000000000000) {
            printf("Numero %lld excedeu limite de 1 trilhão de iteracoes sem convergir. ultimo valor = %lld\n", n, atual);
            break;
        }
    }

    totalPassos += passos;
    totalNumeros++;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <inicio> <fim>\n", argv[0]);
        return 1;
    }

    long long inicio = atoll(argv[1]);
    long long fim = atoll(argv[2]);

    for (long long i = inicio; i <= fim; i++) {
        analisar(i);
        if (i % 1000000 == 0) {
            printf("Analisados %lld numeros ate agora...\n", i);
        }
    }

    printf("Análise concluída de %lld a %lld\n", inicio, fim);
    printf("Total de passos: %lld\n", totalPassos);
    if (totalNumeros > 0)
        printf("Passos médios por número: %.2f\n", (double)totalPassos / totalNumeros);

    return 0;
}

// Para compilar: gcc -O3 EstatisticaTri.c -o EstatisticaTri
// Para rodar: ./EstatisticaTri 1 100000
