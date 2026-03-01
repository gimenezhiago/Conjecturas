#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <gmp.h>

bool ehPrimo(mpz_t n) {
    if (mpz_cmp_si(n, 1) <= 0) return false;
    if (mpz_cmp_si(n, 2) == 0) return true;
    if (mpz_divisible_ui_p(n, 2)) return false;

    mpz_t i, limite, resto;
    mpz_init(i);
    mpz_init(limite);
    mpz_init(resto);

    mpz_sqrt(limite, n);
    for (mpz_set_ui(i, 3); mpz_cmp(i, limite) <= 0; mpz_add_ui(i, i, 2)) {
        mpz_mod(resto, n, i);
        if (mpz_cmp_si(resto, 0) == 0) {
            mpz_clear(i); mpz_clear(limite); mpz_clear(resto);
            return false;
        }
    }
    mpz_clear(i); mpz_clear(limite); mpz_clear(resto);
    return true;
}

void somaAlgarismo(mpz_t r, mpz_t n) {
    mpz_set_ui(r, 0);
    mpz_t dez, resto;
    mpz_init(dez);
    mpz_init(resto);
    mpz_set_ui(dez, 10);
    while (mpz_cmp_ui(n, 0) > 0) {
        mpz_mod(resto, n, dez);
        mpz_add(r, r, resto);
        mpz_divexact(n, n, dez);
    }
    mpz_clear(dez);
    mpz_clear(resto);
}

void aplicarRegrasTri(mpz_t r, mpz_t n) {
    if (mpz_cmp_si(n, 1) <= 0) {
        mpz_set(r, n);
        return;
    }
    if (ehPrimo(n)) {
        mpz_set(r, n);
        return;
    }
    // buscar menor divisor primo
    mpz_t divisor, resto;
    mpz_init(divisor);
    mpz_init(resto);
    mpz_set_ui(divisor, 2);
    while (1) {
        mpz_mod(resto, n, divisor);
        if (mpz_cmp_si(resto, 0) == 0) break;
        // incrementar divisor para próximo primo (simples incremento aqui)
        mpz_add_ui(divisor, divisor, 1);
        while (!ehPrimo(divisor)) mpz_add_ui(divisor, divisor, 1);
        if (mpz_cmp(divisor, n) > 0) break;
    }
    if (mpz_cmp(divisor, n) > 0) {
        mpz_set(r, n);
    } else {
        mpz_fdiv_q(r, n, divisor);
    }
    mpz_clear(divisor);
    mpz_clear(resto);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <numero>\n", argv[0]);
        printf("Testa de 1 até <numero> usando GMP\n");
        return 1;
    }

    long long fim = atoll(argv[1]);
    if (fim <= 0) {
        printf("Parâmetro inválido: %s\n", argv[1]);
        return 1;
    }

    for (long long i = 1; i <= fim; i++) {
        mpz_t atual, proximo;
        mpz_init(atual);
        mpz_t t1, t2;
        mpz_init_set_ui(t1, (unsigned long)i);
        mpz_init_set_ui(t2, (unsigned long)(i + 1));
        // calcular triangular(i)
        mpz_mul_ui(t1, t1, (unsigned long)(i + 1));
        mpz_fdiv_q_ui(t1, t1, 2);
        // calcular triangular(i+1)
        mpz_mul_ui(t2, t2, (unsigned long)(i + 2));
        mpz_fdiv_q_ui(t2, t2, 2);
        // produto dos dois triangulares
        mpz_mul(atual, t1, t2);
        mpz_clear(t1);
        mpz_clear(t2);

        mpz_init(proximo);

        while (1) {
            if (ehPrimo(atual)) break;
            aplicarRegrasTri(proximo, atual);
            mpz_set(atual, proximo);
        }

        mpz_clear(atual);
        mpz_clear(proximo);
    }

    printf("Intervalo concluído.\n");
    return 0;
}

// Para compilar: gcc -O3 TesteIntervaloTriGMP.c -o TesteIntervaloTriGMP -lgmp
// Para rodar: ./TesteIntervaloTriGMP 100
