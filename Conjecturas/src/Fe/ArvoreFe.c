#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

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

void encontrarPreimagens(long long y, long long limite) {
    printf("Pre-imagens de %lld no intervalo [1, %lld]:\n", y, limite);

    long long count = 0;

    for (long long x = 1; x <= limite; x++) {
        if (aplicarRegras(x) == y) {
            printf("%lld -> %lld\n", x, y);
            count++;
        }
    }

    if (count == 0) {
        printf("Nenhuma pre-imagem encontrada para %lld no intervalo especificado.\n", y);
        printf("\n");
    } else {
        printf("Total de pre-imagens encontradas: %lld\n\n", count);
    }
}

void arvore(long long limite, int profundidade) {
    // Para a Conjectura Fe, começamos com o valor 1 (o único ponto fixo)
    long long *nivelAtual = malloc(sizeof(long long) * 1000000);
    long long *nivelProximo = malloc(sizeof(long long) * 1000000);
    long long tamanhoAtual = 1;
    long long tamanhoProximo = 0;

    nivelAtual[0] = 1;

    printf("Ponto de convergencia: 1\n");
    printf("============================================\n");

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
            printf("Nenhuma pre-imagem encontrada na profundidade %d.\n", d);
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

    printf("Construindo arvore de pre-imagens ate %lld com profundidade %d\n", limite, profundidade);
    printf("============================================\n");
    arvore(limite, profundidade);

    return 0;
}

// Para compilar: gcc -O3 ArvoreFe.c -o ArvoreFe
// Para rodar: ./ArvoreFe 100 3