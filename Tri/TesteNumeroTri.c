nclude <stdbool.h> //para booleano
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
}#include <stdio.h>
#i

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

    long long divisor = 2;
    int passos = 0;

    while(true) {
        if (atual % divisor == 0) {
            atual /= divisor;
             printf(" / %lld = %lld", divisor, atual);
            passos++;

            if (passos % 10 == 0) {
                printf("\n");
            }

            if (ehPrimo(atual)) {
                printf("\n\n>>> RESULTADO: %lld (PRIMO) <<<\n", atual);
                return;
            }

            if (ehQuadradoPrimo(atual)) {
                long long raiz = (long long)sqrt(atual);
                printf("\n\n>>> RESULTADO: %lld = %lld² (QUADRADO DE PRIMO) <<<\n", atual, raiz)
                return;
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
        printf("Exemplo: %s 6\n", argv[0]);
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

