#include <stdio.h>
#include <stdbool.h> //Para usar booleano
#include <stdlib.h> //Para usar atoll
#include <time.h> //Para medir tempo

bool ehPrimo(long long n) {
    if (n <= 1) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    for (long long i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

long long somaAlgarismo(long long n) { //inline - troca a funcao por seu conteudo
    long long soma = 0;
    for (; n > 0; n /= 10) { 
        soma += n % 10;
    }
    return soma;
}

long long aplicarRegras(long long n) {
    if (ehPrimo(n)) {
        return n + (n + 1);
    } else if (n % 5 == 0) {
        return n / 5;
    } else if (n % 9 == 0) {
        return n + 1;
    } else if (n % 2 == 0) {
        return n / 2;
    } else {
        return somaAlgarismo(n) + n;
    }
}

void encontrarPreimagens(long long y, long long limite) {
    printf("Pré-imagens de %lld no intervalo [1, %lld]:\n", y, limite);

    long long count = 0;

    for (long long x = 1; x <= limite; x++) {
        if (aplicarRegras(x) == y) {
            printf("%lld - %lld\n", x, y);
            count++;
        }
    }

    if (count == 0) {
        printf("Nenhuma pré-imagem encontrada para %lld no intervalo especificado.\n", y);
        printf("\n");
    }
}

void arvore(long long limite, int profundidade) {
    long long ciclo[3] = {3, 7, 15};

    long long *nivelAtual = malloc(sizeof(long long) * 1000000);
    long long *nivelProximo = malloc(sizeof(long long) * 1000000);
    long long tamanhoAtual = 3;
    long long tamanhoProximo = 0;

    nivelAtual[0] = 3;
    nivelAtual[1] = 7;
    nivelAtual[2] = 15;

    for (int d = 1; d <= profundidade; d++) {
        printf("Profundidade %d:\n", d);

        tamanhoProximo = 0;

        for (long long i = 0; i < tamanhoAtual; i++) {
            long long y = nivelAtual[i];
            for (long long x = 1; x <= limite; x++) {
                if (aplicarRegras(x) == y) {
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

    printf("Construindo árvore de pré-imagens até %lld com profundidade %d\n", limite, profundidade);
    printf("============================================\n");
    arvore(limite, profundidade);

    return 0;
}

// Para compilar: gcc -O3 ArvoreManso.c -o ArvoreManso
// Para rodar: ./ArvoreManso 100 3