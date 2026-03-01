#include <stdio.h>
#include <stdbool.h> //Para usar booleano
#include <stdlib.h> //Para usar atoll
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

// aplica a regra TRI: divide pelo menor primo divisor
long long aplicarRegrasTri(long long n) {
    if (n <= 1) return n;
    long long divisor = 2;
    while (divisor <= n && n % divisor != 0) {
        divisor = proximoPrimo(divisor);
    }
    if (divisor > n) return n; // nenhum divisor encontrado
    return n / divisor;
}

// função auxiliar para calcular o produto de dois triangulares
long long triangular(int m) {
    return (long long)m * (m + 1) / 2;
}

void encontrarPreimagens(long long y, long long limite) {
    printf("Pré-imagens de %lld no intervalo [1, %lld]:\n", y, limite);

    long long count = 0;

    for (long long x = 1; x <= limite; x++) {
        // in Tri conjecture we start com produto de triangulares
        long long atual = triangular(x) * triangular(x + 1);
        // aplicar uma vez para comparar com y
        if (aplicarRegrasTri(atual) == y) {
            printf("%lld - %lld (produto T%lld*T%lld)\n", x, y, x, x+1);
            count++;
        }
    }

    if (count == 0) {
        printf("Nenhuma pré-imagem encontrada para %lld no intervalo especificado.\n", y);
        printf("\n");
    }
}

void arvore(long long limite, int profundidade) {
    // para Tri não há ciclo conhecido, mas podemos iniciar com alguns valores
    long long ciclo[1] = {0}; // não utilizado

    long long *nivelAtual = malloc(sizeof(long long) * 1000000);
    long long *nivelProximo = malloc(sizeof(long long) * 1000000);
    long long tamanhoAtual = 0;
    long long tamanhoProximo = 0;

    // nível 0 não possui valores fixos

    for (int d = 1; d <= profundidade; d++) {
        printf("Profundidade %d:\n", d);

        tamanhoProximo = 0;

        for (long long i = 0; i < tamanhoAtual; i++) {
            long long y = nivelAtual[i];
            for (long long x = 1; x <= limite; x++) {
                long long atual = triangular(x) * triangular(x + 1);
                if (aplicarRegrasTri(atual) == y) {
                    printf("%lld -> %lld\n", x, y);
                    nivelProximo[tamanhoProximo++] = x;
                }
            }
        }
        if (tamanhoProximo == 0) {
            printf("Nenhuma pré-imagem encontrada na profundidade %d.\n", profundidade);
            break;
        }

        long long *tmp = nivelAtual;
        nivelAtual = nivelProximo;
        nivelProximo = tmp;

        tamanhoAtual = tamanhoProximo;

        printf("Total de nos neste nivel: %lld\n", tamanhoAtual);
        printf("\n");
    }

    free(nivelAtual);
    free(nivelProximo);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <limite> <profundidade>\n", argv[0]);
        return 1;
    }

    long long limite = atoll(argv[1]);
    int profundidade = atoi(argv[2]);

    if (limite <= 0 || profundidade <= 0) {
        printf("Parametros invalidos: %s %s\n", argv[1], argv[2]);
        return 1;
    }

    printf("Construindo árvore de pré-imagens Tri até %lld com profundidade %d\n", limite, profundidade);
    printf("============================================\n");
    arvore(limite, profundidade);

    return 0;
}

// Para compilar: gcc -O3 ArvoreTri.c -o ArvoreTri
// Para rodar: ./ArvoreTri 100 3
