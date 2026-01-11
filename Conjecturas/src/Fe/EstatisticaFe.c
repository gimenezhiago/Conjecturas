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

int aplicarRegraIdentificada(long long *n) {
    long long x = *n;

    if (x % 2 == 0) {
        *n = x / 2;
        return 1; // PAR
    }
    if (x % 3 == 0) {
        *n = x / 3;
        return 2; // DIVISÍVEL POR 3
    }
    if (x % 5 == 0) {
        *n = x / 5;
        return 3; // DIVISÍVEL POR 5
    }
    if (x % 7 == 0) {
        *n = x / 7;
        return 4; // DIVISÍVEL POR 7
    }

    *n = x + somaDigitos(x);
    return 5; // SOMA DOS DÍGITOS
}

long long totalPar = 0;
long long totalDiv3 = 0;
long long totalDiv5 = 0;
long long totalDiv7 = 0;
long long totalSoma = 0;
long long totalPassos = 0;

void analisar(long long n) {
    long long atual = n;

    while (1) {
        if (atual == 1) {
            break;
        }

        int regra = aplicarRegraIdentificada(&atual);

        if (regra == 1) totalPar++;
        else if (regra == 2) totalDiv3++;
        else if (regra == 3) totalDiv5++;
        else if (regra == 4) totalDiv7++;
        else if (regra == 5) totalSoma++;

        totalPassos++;

        if (totalPassos > 1000000000000LL) {
            printf("Numero %lld excedeu o limite de 1 trilhao de iteracoes sem convergir. Ultimo valor = %lld\n", n, atual);
            return;
        }
    }
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

    printf("Analise concluida ate %lld\n", fim);
    printf("Total de passos: %lld\n", totalPassos);
    printf("Total de aplicacoes da regra PAR: %lld\n", totalPar);
    printf("Total de aplicacoes da regra DIVISIVEL POR 3: %lld\n", totalDiv3);
    printf("Total de aplicacoes da regra DIVISIVEL POR 5: %lld\n", totalDiv5);
    printf("Total de aplicacoes da regra DIVISIVEL POR 7: %lld\n", totalDiv7);
    printf("Total de aplicacoes da regra SOMA DOS DIGITOS: %lld\n", totalSoma);
    printf("============================================\n");
    printf("Porcentagem PAR: %.2f%%\n", (double)totalPar * 100.0 / totalPassos);
    printf("Porcentagem DIVISIVEL POR 3: %.2f%%\n", (double)totalDiv3 * 100.0 / totalPassos);
    printf("Porcentagem DIVISIVEL POR 5: %.2f%%\n", (double)totalDiv5 * 100.0 / totalPassos);
    printf("Porcentagem DIVISIVEL POR 7: %.2f%%\n", (double)totalDiv7 * 100.0 / totalPassos);
    printf("Porcentagem SOMA DOS DIGITOS: %.2f%%\n", (double)totalSoma * 100.0 / totalPassos);

    return 0;
}

// Para compilar: gcc -O3 EstatisticaFe.c -o EstatisticaFe
// Para rodar: ./EstatisticaFe 1 100000