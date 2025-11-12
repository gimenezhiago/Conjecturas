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

void testeNumero (long long fim) {
    const long long LIMITE = 1000000000000;
    long long atual = n;
    long long contador = 0;


        while (contador < LIMITE) {
            if (atual == 1) {
                printf("Convergiu em %lld passos\n", contador);
                return;
            }

           atual = aplicarRegras(atual);
            contador++;

            printf("%lld - ", atual);

            if (contador % 10 == 0) {
                printf("\n");
            }
        }

           printf("\nO numero %lld nao convergiu (>%lld iteracoes). ultimo valor = %lld\n", n, LIMITE, atual);

    }


int main (int argc, char *argv[]) {
    if (argc < 2) { //verifica se tem o parametro
        printf("Uso: %s <numero>\n", argv[0]);
        return 1;
    }

    long long numero = atoll(argv[1]); //converte string para inteiro
    if (numero <= 0) {
        printf("Parametro invalido: %s\n", argv[1]);
        return 1;
    }

    testarNumero(numero);

    return 0;
}


// Para compilar: gcc -O3 TesteNumeroFe.c -o TesteNumeroFe
// Para rodar: ./TesteNumeroFe 100