#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <gmp.h>

void somaDigitos(mpz_t resultado, mpz_t n) {
    mpz_t temp;
    mpz_init(temp);
    mpz_set(temp, n);

    mpz_set_ui(resultado, 0);
    while (mpz_cmp_ui(temp, 0) > 0) {
        unsigned long d = mpz_fdiv_ui(temp, 10); // resto temp % 10
        mpz_add_ui(resultado, resultado, d);
        mpz_tdiv_q_ui(temp, temp, 10); // temp /= 10
    }

    mpz_clear(temp);
}

void aplicarRegras(mpz_t resultado, mpz_t n) {
    mpz_t soma;
    mpz_init(soma);

    if (mpz_divisible_ui_p(n, 2)) { // n % 2 == 0
        mpz_tdiv_q_ui(resultado, n, 2);
    } else if (mpz_divisible_ui_p(n, 3)) { // n % 3 == 0
        mpz_tdiv_q_ui(resultado, n, 3);
    } else if (mpz_divisible_ui_p(n, 5)) { // n % 5 == 0
        mpz_tdiv_q_ui(resultado, n, 5);
    } else if (mpz_divisible_ui_p(n, 7)) { // n % 7 == 0
        mpz_tdiv_q_ui(resultado, n, 7);
    } else {
        somaDigitos(soma, n);
        mpz_add(resultado, n, soma); // resultado = n + somaDigitos(n)
    }

    mpz_clear(soma);
}

void testeIntervalo(long long fim) {
    mpz_t i, atual, contador, LIMITE;
    mpz_init(i);
    mpz_init(atual);
    mpz_init(contador);
    mpz_init(LIMITE);

    mpz_set_ui(LIMITE, 1000000000000LL); // 1 trilhão

    for (mpz_set_ui(i, 1); mpz_cmp_ui(i, fim) <= 0; mpz_add_ui(i, i, 1)) {
        mpz_set(atual, i); // atual = i
        mpz_set_ui(contador, 0); // contador = 0
        bool convergiu = false;

        while (mpz_cmp(contador, LIMITE) < 0) {
            if (mpz_cmp_ui(atual, 1) == 0) {
                convergiu = true;
                break;
            }

            aplicarRegras(atual, atual); // atual = aplicarRegras(atual)
            mpz_add_ui(contador, contador, 1); // contador++

            if (mpz_cmp_ui(atual, 0) <= 0) {
                gmp_printf("O numero %Zd gerou valor invalido/overflow em %Zd iteracoes. ultimo valor = %Zd\n", 
                          i, contador, atual);
                break;
            }
        }

        if (!convergiu) {
            gmp_printf("O numero %Zd nao convergiu (>%Zd iteracoes). ultimo valor = %Zd\n", 
                      i, LIMITE, atual);
            break;
        }

        if (mpz_divisible_ui_p(i, 100000)) {
            gmp_printf("Testados %Zd numeros\n", i);
        }
    }

    gmp_printf("Teste concluido ate %lld\n", fim);

    mpz_clear(i);
    mpz_clear(atual);
    mpz_clear(contador);
    mpz_clear(LIMITE);
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

    printf("Testando ate %lld (usando GMP para precisao arbitraria)\n", fim);
    testeIntervalo(fim);

    return 0;
}

// Para compilar: gcc -O3 TesteIntervaloFeGMP.c -o TesteIntervaloFeGMP -lgmp
// Para rodar: ./TesteIntervaloFeGMP 100
