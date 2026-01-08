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

void analisarNumero(long long n) {
    long long inicial = n;
    long long atual = n;
    long long maximo = n;
    long long iteracoes = 0;
    const long long LIMITE = 1000000000000LL; // 1 trilhão

    while (1) {
        if (atual == 1) {
            break;
        }

        long long proximo = aplicarRegras(atual);
        iteracoes++;

        if (iteracoes > LIMITE) {
            printf("Numero %lld excedeu o limite de %lld iteracoes sem convergir. Ultimo valor = %lld\n", inicial, LIMITE, atual);
            return;
        }

        if (proximo < 0) {
            printf("Numero %lld gerou valor invalido/overflow em %lld iteracoes. Ultimo valor = %lld\n", inicial, iteracoes, proximo);
            return;
        }

        if (proximo > maximo) {
            maximo = proximo;
        }

        atual = proximo;
    }

    double fator = (double)maximo / (double)inicial;

    // Critério para números "difíceis": crescimento significativo ou muitas iterações
    if (maximo > inicial * 1000 || iteracoes > 100) {
        printf("\n=== Numero Dificil ===\n");
        printf("Numero Inicial: %lld\n", inicial);
        printf("Maior valor: %lld\n", maximo);
        printf("Iteracoes: %lld\n", iteracoes);
        printf("Fator de Crescimento: %.2f\n", fator);
        printf("=====================\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <inicio> <fim>\n", argv[0]);
        return 1;
    }

    long long inicio = atoll(argv[1]);
    long long fim = atoll(argv[2]);

    printf("Analisando numeros de %lld ate %lld\n", inicio, fim);
    printf("============================================\n");

    for (long long n = inicio; n <= fim; n++) {
        analisarNumero(n);

        if (n % 1000000 == 0) {
            printf("Analisados ate: %lld\n", n);
        }
    }

    printf("\nAnalise concluida!\n");

    return 0;
}

// Para compilar: gcc -O3 MaiorNumeroFe.c -o MaiorNumeroFe
// Para rodar: ./MaiorNumeroFe 1 10000