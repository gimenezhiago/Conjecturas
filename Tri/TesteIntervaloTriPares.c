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

bool testarPrimalidade(int n, bool verbose) {
    long long t1 = triangular(n);
    long long t2 = triangular(n + 1);
    long long produto = t1 * t2;
    long long atual = produto;
    
    if (verbose) {
        printf("\n=== Testando T%d x T%d ===\n", n, n + 1);
        printf("T%d = %lld, T%d = %lld\n", n, t1, n + 1, t2);
        printf("Produto: %lld x %lld = %lld\n", t1, t2, produto);
        printf("\nDivisões:\n");
        printf("%lld", atual);
    }
    
    if (ehPrimo(atual)) {
        if (verbose) {
            printf("\n\n>>> RESULTADO: %lld (PRIMO) <<<\n", atual);
            printf("Conclusão: %d é PRIMO\n", n + 1);
        }
        return true; // (n+1) é primo
    }

    long long divisor = 2;
    int passos = 0;
    long long limite_divisor = (long long)sqrt(n + 1);
    
    while (true) {
        if (atual % divisor == 0) {
            atual = atual / divisor;
            if (verbose) {
                printf(" / %lld = %lld", divisor, atual);
                passos++;
                
                if (passos % 10 == 0) {
                    printf("\n");
                }
            }
            
            if (ehPrimo(atual)) {
                if (verbose) {
                    printf("\n\n>>> RESULTADO: %lld (PRIMO) <<<\n", atual);
                    printf("Conclusão: %d é COMPOSTO\n", n + 1);
                }
                return false; // (n+1) é composto
            }
            
            if (ehQuadradoPrimo(atual)) {
                long long raiz = (long long)sqrt(atual);

                if (raiz > limite_divisor) {
                    if (verbose) {
                        printf("\n\n>>> RESULTADO: %lld = %lld² (QUADRADO DE PRIMO) <<<\n", atual, raiz);
                        printf("Conclusão: %d é PRIMO\n", n + 1);
                    }
                    return true; // (n+1) é primo
                } else {
                    if (verbose) {
                        printf(" [quadrado %lld² mas continua]", raiz);
                    }
                    continue;
                }
            }
        } else {
            divisor = proximoPrimo(divisor);
            
            if (divisor > atual) {
                if (verbose) {
                    printf("\n\nERRO: Divisor maior que o número atual!\n");
                    printf("Resultado final: %lld\n", atual);
                }
                return false; // Assume composto em caso de erro
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <quantidade_primos> [verbose]\n", argv[0]);
        printf("Gera lista de primos usando o teste de triangulares\n");
        return 1;
    }
    
    int qtd_primos = atoi(argv[1]);
    bool verbose = (argc >= 3 && strcmp(argv[2], "verbose") == 0);
    
    if (qtd_primos <= 0) {
        printf("Parâmetro inválido: %s\n", argv[1]);
        printf("A quantidade deve ser maior que 0\n");
        return 1;
    }
    
    printf("Gerando os %d primeiros primos...\n\n", qtd_primos);
    
    int *primos = (int *)malloc(qtd_primos * sizeof(int)); 
    int contador = 0;
    
    // Adiciona o 2 como primeiro primo
    primos[contador++] = 2;
    if (verbose) {
        printf("Primo #1: 2 (dado)\n");
    }
 
    int n = 2;
    
    while (contador < qtd_primos) {
        int candidato = n + 1; // O número sendo testado
        
        bool eh_primo_result = testarPrimalidade(n, verbose);
        
        if (eh_primo_result) {
            primos[contador] = candidato;
            contador++;
            if (verbose) {
                printf("\n>>> Primo #%d encontrado: %d <<<\n", contador, candidato);
            }
        }
        
        n += 2; // Avança para o próximo n par
    }

    printf("LISTA DOS %d PRIMEIROS PRIMOS:\n", qtd_primos);

    for (int i = 0; i < qtd_primos; i++) {
        printf("%4d", primos[i]);
        if ((i + 1) % 10 == 0) {
            printf("\n");
        } else {
            printf(" ");
        }
    }
    if (qtd_primos % 10 != 0) {
        printf("\n");
    }
    
    printf("O %dº primo é: %d\n", qtd_primos, primos[qtd_primos - 1]);
    
    printf("\nVerificando consistência...\n");
    bool correto = true;
    for (int i = 0; i < qtd_primos; i++) {
        if (!ehPrimo(primos[i])) {
            printf("ERRO: %d não é primo!\n", primos[i]);
            correto = false;
        }
    }
    
    if (correto) {
        printf("Todos os números da lista são primos!\n");
    }
    
    free(primos);
    
    return 0;
}

// Para compilar: gcc -O3 TesteIntervaloTri.c -o TesteIntervaloTri -lm
// Para rodar: ./TesteIntervaloTri 1000
// Para ver detalhes: ./TesteIntervaloTri 10 verbose