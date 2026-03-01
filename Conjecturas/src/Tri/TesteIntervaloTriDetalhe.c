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

void testeIntervalo(long long fim) {
    const long long LIMITE = 1000000000000; //1 trilhão
    for (long long i = 1; i <= fim; i++) {
        long long atual = triangular(i) * triangular(i + 1);
        long long contador = 0;
        bool convergiu = false;
        long long valorFinal = 0;

        while (contador < LIMITE) {
            if (ehPrimo(atual) || ehQuadradoPrimo(atual)) {
                convergiu = true;
                valorFinal = atual;
                break;
            }

            atual = aplicarRegrasTri(atual);
            contador++;

            if (atual <= 0) { // checagem simples para overflow/valor inválido
                printf("O numero %lld gerou valor invalido/overflow em %lld iteracoes. ultimo valor = %lld\n", i, contador, atual);
                break;
            }
        }

        if (convergiu) {
            printf("%lld -> %lld em %lld passos\n", i, valorFinal, contador);
        } else {
            printf("O numero %lld nao convergiu (> %lld iteracoes). ultimo valor = %lld\n", i, LIMITE, atual);
            break;
        }

    }
    printf("Teste concluido ate %lld\n", fim);
}

int main(int argc, char *argv[]) {
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
    printf("============================================\n");
    testeIntervalo(fim);

    return 0;
}

// Para compilar: gcc -O3 TesteIntervaloTriDetalhe.c -o TesteIntervaloTriDetalhe
// Para rodar: ./TesteIntervaloTriDetalhe 100
