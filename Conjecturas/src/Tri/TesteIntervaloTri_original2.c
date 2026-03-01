#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
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
    
    if (ehPrimo(atual)) {
        printf("\n\n>>> RESULTADO: %lld (PRIMO) <<<\n", atual);
        if (n % 2 == 0) {
            printf("Conclusão: %d é PRIMO\n", n + 1);
        }
        return;
    }

    long long divisor = 2;
    int passos = 0;

    long long limite_divisor = 0;
    if (n % 2 == 0) {
        limite_divisor = (long long)sqrt(n + 1) + 1;
    }
    
    while (true) {
        if (atual % divisor == 0) {
            atual = atual / divisor;
            printf(" / %lld = %lld", divisor, atual);
            passos++;
            
            if (passos % 10 == 0) {
                printf("\n");
            }
            
            // Verifica se chegou a um primo ou quadrado de primo
            if (ehPrimo(atual)) {
                printf("\n\n>>> RESULTADO: %lld (PRIMO) <<<\n", atual);
                if (n % 2 == 0) {
                    printf("Conclusão: %d é COMPOSTO\n", n + 1);
                }
                return;
            }
            
            if (ehQuadradoPrimo(atual)) {
                long long raiz = (long long)sqrt(atual);

                if (n % 2 == 0) {

                    if (raiz < limite_divisor) {
                        printf(" [quadrado %lld² mas continua dividindo]", raiz);
                        continue;
                    } else {
                        printf("\n\n>>> RESULTADO: %lld = %lld² (QUADRADO DE PRIMO) <<<\n", atual, raiz);
                        printf("Conclusão: %d é COMPOSTO\n", n + 1);
                        return;

                    }
                } else {
                    printf("\n\n>>> RESULTADO: %lld = %lld² (QUADRADO DE PRIMO) <<<\n", atual, raiz);
                    return;
                }
            }
        } else {
            // Avança para o próximo primo
            divisor = proximoPrimo(divisor);
            
            // Proteção contra loop infinito
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
        printf("Uso: %s <numero>\n", argv[0]);
        printf("Testa T(n) x T(n+1) de n=1 até n=<numero>\n");
        printf("Para n par: testa primalidade de (n+1)\n");
        printf("Para n ímpar: apenas verifica a conjectura\n");
        return 1;
    }
    
    int fim = atoi(argv[1]);
    
    if (fim <= 0) {
        printf("Parâmetro inválido: %s\n", argv[1]);
        printf("O número deve ser maior que 0\n");
        return 1;
    }
    

    printf("Intervalo: n = 1 até %d\n", fim);
    printf("(n par = teste de primalidade; n ímpar = apenas conjectura)\n");
    
    for (int i = 1; i <= fim; i++) {
        testarConjectura(i);
    }
    
    return 0;
}

// Para compilar: gcc -O3 TesteIntervaloTri.c -o TesteIntervaloTri -lm
// Para rodar: ./TesteIntervaloTri 6