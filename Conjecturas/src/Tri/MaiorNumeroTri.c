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

long long calculaMaior(long long limite) {
    long long maior = 0;
    long long passoMaior = 0;

    for (long long i = 1; i <= limite; i++) {
        long long atual = triangular(i) * triangular(i + 1);
        long long passos = 0;

        while (!(ehPrimo(atual) || ehQuadradoPrimo(atual))) {
            passos++;
            atual = aplicarRegrasTri(atual);
            if (passos > maior) {
                maior = passos;
                passoMaior = i;
            }
            if (passos > 1000000000000) break;
        }
    }

    printf("Maior número de passos até %lld foi %lld (começando em %lld)\n", limite, maior, passoMaior);
    return passoMaior;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <limite>\n", argv[0]);
        return 1;
    }

    long long limite = atoll(argv[1]);
    if (limite <= 0) {
        printf("Parâmetro inválido: %s\n", argv[1]);
        return 1;
    }

    calculaMaior(limite);
    return 0;
}

// Para compilar: gcc -O3 MaiorNumeroTri.c -o MaiorNumeroTri
// Para rodar: ./MaiorNumeroTri 100
