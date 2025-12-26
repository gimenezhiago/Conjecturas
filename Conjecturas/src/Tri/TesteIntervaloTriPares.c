// TesteIntervaloTriPares.c
// Corrigido conforme regras do "Fê" (divisões por primos em ordem, testar n pares, incluir 2, ...)

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

bool ehPrimo(long long n) {
    if (n <= 1) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    long long r = (long long)sqrt((double)n);
    for (long long i = 3; i <= r; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

bool ehQuadradoPrimo(long long n, long long *raiz_out) {
    if (n <= 0) return false;
    long long raiz = (long long) llround(sqrt((double)n));
    if (raiz * raiz == n && ehPrimo(raiz)) {
        if (raiz_out) *raiz_out = raiz;
        return true;
    }
    return false;
}

long long triangular(long long n) {
    return n * (n + 1) / 2;
}

long long proximoPrimo(long long n) {
    if (n < 2) return 2;
    long long p = n + 1;
    if (p % 2 == 0 && p != 2) p++;
    while (!ehPrimo(p)) {
        p += 2;
    }
    return p;
}

/*
  testarPrimalidade:
  - recebe n (par)
  - aplica o procedimento do Fê em Tn x T(n+1)
  - retorna true se (n+1) for primo, false se composto
*/
bool testarPrimalidade(int n, bool verbose) {
    long long t1 = triangular(n);
    long long t2 = triangular(n + 1);
    long long produto = t1 * t2;
    long long atual = produto;
    long long alvo = (long long)(n + 1);

    if (verbose) {
        printf("\n=== Testando T%d x T%d ===\n", n, n + 1);
        printf("T%d = %lld, T%d = %lld\n", n, t1, n + 1, t2);
        printf("Produto: %lld x %lld = %lld\n", t1, t2, produto);
        printf("\nDivisões:\n%lld", atual);
    }

    // Se já for primo no começo (caso raro), segundo o Fê concluiríamos que (n+1) é composto?
    // No enunciado, T1xT2 = 3 é primo e não se divide — mas Fê quer que não se divida.
    // A fórmula abaixo segue a regra principal: dividimos por primos < (n+1).
    long long divisor = 2;

    while (divisor < alvo) {
        bool dividiuQualquerVez = false;
        while (atual % divisor == 0) {
            atual /= divisor;
            dividiuQualquerVez = true;
            if (verbose) {
                printf(" / %lld = %lld", divisor, atual);
            }
            // Se durante as divisões o resultado virar primo => concluímos (n+1) é composto
            if (ehPrimo(atual)) {
                if (verbose) {
                    printf("\n\n>>> RESULTADO: %lld (PRIMO) <<<\n", atual);
                    printf("Conclusão: %lld é COMPOSTO\n", alvo);
                }
                return false; // (n+1) é composto
            }
        }
        // Avança para o próximo primo (mantendo-se abaixo de alvo)
        divisor = proximoPrimo(divisor);
        if (divisor >= alvo) break;
    }

    // Ao terminar de testar TODOS os primos < (n+1), analisamos o que sobrou em 'atual'
    if (verbose) {
        printf("\n\nResultado final após testar primos < %lld: %lld\n", alvo, atual);
    }

    if (atual == 1) {
        // tudo foi fatorado por primos < (n+1) -> n+1 é composto
        if (verbose) printf("Conclusão: %lld é COMPOSTO (fatorado completamente por primos menores)\n", alvo);
        return false;
    }

    // Se for quadrado de primo e a raiz for exatamente n+1 => n+1 primo (casos como 169 -> 13^2)
    long long raiz;
    if (ehQuadradoPrimo(atual, &raiz)) {
        if (raiz == alvo) {
            if (verbose) {
                printf(">>> RESULTADO: %lld = %lld^2 (QUADRADO DE PRIMO) <<<\n", atual, raiz);
                printf("Conclusão: %lld é PRIMO\n", alvo);
            }
            return true; // (n+1) é primo
        } else {
            // ex.: se raiz < alvo então deveria ter sido dividida antes => então composto
            if (verbose) {
                printf("Quadrado de primo %lld^2 (raiz %lld) mas raiz != %lld -> CONCLUI: COMPOSTO\n", raiz, raiz, alvo);
            }
            return false;
        }
    }

    // Se o que sobrou é primo e igual a alvo -> conclui primo
    if (ehPrimo(atual)) {
        if (atual == alvo) {
            if (verbose) {
                printf(">>> RESULTADO: %lld (PRIMO) <<<\n", atual);
                printf("Conclusão: %lld é PRIMO\n", alvo);
            }
            return true;
        } else {
            // se o que sobrou é primo diferente de alvo (maior), então concluímos composto
            if (verbose) {
                printf("Sobra primo %lld != %lld -> CONCLUI: COMPOSTO\n", atual, alvo);
            }
            return false;
        }
    }

    // Nenhum dos casos acima: não é 1, não é primo, não é quadrado-primo => composto
    if (verbose) {
        printf("Sobra %lld (não primo, não 1) -> Conclusão: %lld é COMPOSTO\n", atual, alvo);
    }
    return false;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <quantidade_primos> [verbose]\n", argv[0]);
        printf("Gera lista de primos usando o teste de triangulares (apenas n pares)\n");
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
    if (!primos) {
        fprintf(stderr, "Erro: malloc\n");
        return 1;
    }

    int contador = 0;

    // Adiciona o 2 como primeiro primo
    primos[contador++] = 2;
    if (verbose) {
        printf("Primo #1: 2 (adicionado manualmente)\n");
    }

    int n = 2; // começar com n par = 2 -> testa primalidade de n+1 = 3

    while (contador < qtd_primos) {
        int candidato = n + 1; // número cuja primalidade estamos testando
        bool eh_primo_result = testarPrimalidade(n, verbose);

        if (eh_primo_result) {
            primos[contador] = candidato;
            contador++;
            if (verbose) {
                printf("\n>>> Primo #%d encontrado: %d <<<\n", contador, candidato);
            }
        }

        n += 2; // avança para próximo n par
        // prevenir overflow de n (caso usuário peça muitos primos); mas deixei simples
    }

    printf("\nLISTA DOS %d PRIMEIROS PRIMOS:\n", qtd_primos);
    for (int i = 0; i < qtd_primos; i++) {
        printf("%4d", primos[i]);
        if ((i + 1) % 10 == 0) printf("\n");
        else printf(" ");
    }
    if (qtd_primos % 10 != 0) printf("\n");

    printf("O %dº primo é: %d\n", qtd_primos, primos[qtd_primos - 1]);

    printf("\nVerificando consistência...\n");
    bool correto = true;
    for (int i = 0; i < qtd_primos; i++) {
        if (!ehPrimo(primos[i])) {
            printf("ERRO: %d não é primo!\n", primos[i]);
            correto = false;
        }
    }
    if (correto) printf("Todos os números da lista são primos!\n");

    free(primos);
    return 0;
}
