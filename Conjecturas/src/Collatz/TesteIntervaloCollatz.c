#include <stdio.h>
#include <stdbool.h> //Para usar booleano
#include <stdlib.h> //Para usar atoll

bool ehParImpar(long long n) {
    if (n % 2 == 0) {
        return n/2;
    } else {
        return 3*n + 1;
    }
}


long long aplicarRegras(long long n) {
    return ehParImpar(n);
}

void testeIntervalo(long long fim) {
    const long long LIMITE = 1000000000000; //1 trilhão
    for (long long i = 1; i <= fim; i++) {
        long long atual = i;
        long long contador = 0;
        bool convergiu = false;

        while (contador < LIMITE) {
            if (atual == 3 || atual == 7 || atual == 15) {
                convergiu = true;
                break;
            }

            atual = aplicarRegras(atual);
            contador++;

            if (atual <= 0) { // checagem simples para overflow/valor inválido
                printf("O numero %lld gerou valor invalido/overflow em %lld iteracoes. ultimo valor = %lld\n", i, contador, atual);
                break;
            }
        }

        if (!convergiu) {
            printf("O numero %lld nao convergiu (>%lld iteracoes). ultimo valor = %lld\n", i, LIMITE, atual);
            break;
        }

        if (i % 100000 == 0) {
            printf("Testados %lld numeros\n", i);
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
    testeIntervalo(fim);

    return 0;
}

// Para compilar: gcc -O3 TesteIntervaloManso.c -o TesteIntervaloManso
// Para rodar: ./TesteIntervaloManso 100