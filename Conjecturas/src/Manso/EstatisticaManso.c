#include <stdio.h>
#include <stdbool.h> //Para usar booleano
#include <stdlib.h> //Para usar atoll

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

int aplicarRegraIdentificada(long long *n) {
    long long x = *n;

    if (ehPrimo(x)) {
        *n = x + (x + 1);
        return 1; // PRIMO
    }
    if (x % 5 == 0) {
        *n = x / 5;
        return 2; // DIVISÍVEL POR 5
    }
    if (x % 9 == 0) {
        *n = x + 1;
        return 3; // DIVISÍVEL POR 9
    }
    if (x % 2 == 0) {
        *n = x / 2;
        return 4; // PAR
    }

    *n = x + somaAlgarismo(x);
    return 5; // SOMA DOS ALGARISMOS
}

long long totalPrimo = 0;
long long totalDiv5 = 0;
long long totalDiv9 = 0;
long long totalPar = 0;
long long totalSoma = 0;
long long totalPassos = 0;

void analisar(long long n) {
    long long atual = n;

    while (1) {
        if (atual == 3 || atual == 7 || atual == 15) {
            break;
        }

        int regra = aplicarRegraIdentificada(&atual); 

        if (regra == 1) totalPrimo++;
        else if (regra == 2) totalDiv5++;
        else if (regra == 3) totalDiv9++;
        else if (regra == 4) totalPar++;
        else if (regra == 5) totalSoma++;

        totalPassos++;

        if (totalPassos > 1000000000000) {
            printf("Numero %lld excedeu o limite de 1 trilhão de iteracoes sem convergir. Ultimo valor = %lld\n", n, atual);
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

    for (long long i = 1; i <= fim; i++) {
        analisar(i);

        if (i % 1000000 == 0) {
            printf("Analisados %lld numeros ate agora...\n", i);
        }
    }

    printf("Análise concluída até %lld\n", fim);
    printf("Total de passos: %lld\n", totalPassos);
    printf("Total de aplicações da regra PRIMO: %lld\n", totalPrimo);
    printf("Total de aplicações da regra DIVISÍVEL POR 5: %lld\n", totalDiv5);
    printf("Total de aplicações da regra DIVISÍVEL POR 9: %lld\n", totalDiv9);
    printf("Total de aplicações da regra PAR: %lld\n", totalPar);
    printf("Total de aplicações da regra SOMA DOS ALGARISMOS: %lld\n", totalSoma);
    printf("============================================\n");
    printf("Porcentagem PRIMO: %.2f%%\n", (double)totalPrimo * 100.0 / totalPassos);
    printf("Porcentagem DIVISÍVEL POR 5: %.2f%%\n", (double)totalDiv5 * 100.0 / totalPassos);
    printf("Porcentagem DIVISÍVEL POR 9: %.2f%%\n", (double)totalDiv9 * 100.0 / totalPassos);
    printf("Porcentagem PAR: %.2f%%\n", (double)totalPar * 100.0 / totalPassos);
    printf("Porcentagem SOMA DOS ALGARISMOS: %.2f%%\n", (double)totalSoma * 100.0 / totalPassos);

    return 0;
}

// Para compilar: gcc -O3 EstatisticaManso.c -o EstatisticaManso
// Para rodar: ./EstatisticaManso 1 100000