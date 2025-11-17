#include <stdio.h>
#include <stdbool.h> //para booleano
#include <stdlib.h> //serve para pegar o atoi
#include <math.h> //para sqrt

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
    long long raiz = (long long)sqrt(n); // calcula a raiz quadrada inteira
    if (raiz * raiz == n && ehPrimo(raiz)) {
        return true;
    }
    return false;
}

long long triangular(int n) {
    return (long long)n * (n + 1) / 2;
}

long long proximoPrimo(long long n) {
    long long p = n + 1;
    while (!ehPrimo(p)) {
        p++;
    }
    return p;
}

void testarConjectura(int n) {
    long long t1 = triangular(n);
    long long t2 = triangular(n + 1);
    long long produto = t1 * t2;
    long long atual = produto;

    printf("\n=== Testando T%d x T%d ===\n", n, n + 1);
    printf("T%d = %lld, T%d = %lld\n", n, t1, n + 1, t2);
    printf("Produto: %lld x %lld = %lld\n", t1, t2, produto);
    printf("\nDivisões:\n");
    printf("%lld", atual);

    // Caso especial: se o produto inicial já é primo, não divide
    if (ehPrimo(atual)) {
        printf("\n\n>>> RESULTADO: %lld (PRIMO) <<<\n", atual);
        if (n % 2 == 0) {
            printf("Conclusão: %d é PRIMO\n", n + 1);
        }
        return;
    }

    long long divisor = 2;
    int passos = 0;

    // Para n par (teste de primalidade), calculamos o limite de divisores
    long long limite_divisor = 0;
    if (n % 2 == 0) {
        // Limite é aproximadamente sqrt(n+1)
        limite_divisor = (long long)sqrt(n + 1) + 1;
    }

    while(true) {
        if (atual % divisor == 0) {
            atual /= divisor;
            printf(" / %lld = %lld", divisor, atual);
            passos++;

            if (passos % 10 == 0) {
                printf("\n");
            }

            // Verifica se chegou a um primo
            if (ehPrimo(atual)) {
                printf("\n\n>>> RESULTADO: %lld (PRIMO) <<<\n", atual);
                if (n % 2 == 0) {
                    printf("Conclusão: %d é COMPOSTO\n", n + 1);
                }
                return;
            }

            // Verifica se é quadrado de primo
            if (ehQuadradoPrimo(atual)) {
                long long raiz = (long long)sqrt(atual);

                // Se n é par (teste de primalidade)
                if (n % 2 == 0) {
                    // Verifica se a raiz é menor que sqrt(n+1)
                    if (raiz < limite_divisor) {
                        // Quadrado é divisível por primo menor que sqrt(n+1)
                        // Continua dividindo
                        printf(" [quadrado %lld² mas continua]", raiz);
                        continue;
                    } else {
                        // Quadrado não é divisível por primos menores que sqrt(n+1)
                        printf("\n\n>>> RESULTADO: %lld = %lld² (QUADRADO DE PRIMO) <<<\n", atual, raiz);
                        printf("Conclusão: %d é PRIMO\n", n + 1);
                        return;
                    }
                } else {
                    // n ímpar - apenas verifica a conjectura
                    printf("\n\n>>> RESULTADO: %lld = %lld² (QUADRADO DE PRIMO) <<<\n", atual, raiz);
                    return;
                }
            }

        } else {
            divisor = proximoPrimo(divisor);

            if (divisor > atual) {
                printf("\n\nERRO: Divisor maior que o número atual!\n");
                printf("Resultado final: %lld\n", atual);
                if (ehPrimo(atual)) {
                    printf("(É primo)\n");
                } else if (ehQuadradoPrimo(atual)) {
                    long long raiz = (long long)sqrt(atual);
                    printf("(É %lld²)\n", raiz);
                } else {
                    printf("(NÃO é primo nem quadrado de primo!)\n");
                }
                return;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <n>\n", argv[0]);
        printf("Testa T(n) x T(n+1)\n");
        printf("Para n par: testa primalidade de (n+1)\n");
        printf("Para n ímpar: apenas verifica a conjectura\n");
        return 1;
    }
    
    int n = atoi(argv[1]);
    
    if (n <= 0) {
        printf("Parâmetro inválido: %s\n", argv[1]);
        return 1;
    }

    testarConjectura(n);

    return 0;
}

// Para compilar: gcc -O3 TesteNumeroTri.c -o TesteNumeroTri -lm
// Para rodar: ./TesteNumeroTri 6