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

long long aplicarRegras(long long n) {
    if (ehPrimo(n)) {
        return n + (n + 1);
    } else if (n % 5 == 0) {
        return n / 5;
    } else if (n % 9 == 0) {
        return n + 1;
    } else if (n % 2 == 0) {
        return n / 2;
    } else {
        return somaAlgarismo(n) + n;
    }
}

void analisarNumero(long long n) {
    long long inicial = n;
    long long atual = n;

    long long maximo = n;
    long long iteracoes = 0;
    const long long LIMITE = 1000000000000; //1 trilhão

    while (1) {
        if (atual == 3 || atual == 7 || atual == 15) {
            break;
        }

        long long proximo = aplicarRegras(atual);
        iteracoes++;

        if (iteracoes > LIMITE) {
            printf("Numero %lld excedeu o limite de %lld iteracoes sem convergir. Ultimo valor = %lld\n", inicial, LIMITE, atual);
            return;
        }

        if (proximo < 0) {
            printf("Numero %lld gerou valor invalido/overflow em %lld iteracoes. ultimo valor = %lld\n", inicial, iteracoes, proximo);
            return;
        }

        if (proximo > maximo) {
            maximo = proximo;
        }

        atual = proximo;
    }

    double fator = (double)maximo / (double)inicial; // Cálculo do fator de crescimento

    if (maximo > inicial * 1000 || iteracoes > 1000) { 
        printf("\n=== Número Difícil ===\n");
        printf("Número Inicial: %lld\n", inicial);
        printf("Maior valor: %lld\n", maximo);
        printf("Iterações: %lld\n", iteracoes);
        printf("Fator de Crescimento: %.2f\n", fator);
        printf("=====================\n");

    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <inicio> <fim>\n", argv[0]);
        return 1;
    }

    long long inicio = atoll(argv[1]);
    long long fim = atoll(argv[2]);

    for (long long n = inicio; n <= fim; n++) {
        analisarNumero(n);

        if (n % 1000000 == 0) {
            printf("Analisados ate: %lld\n", n);
        }
    }

    return 0;
}

// Para compilar: gcc -O3 MaiorNumeroManso.c -o MaiorNumeroManso
// Para rodar: ./MaiorNumeroManso 100 3
