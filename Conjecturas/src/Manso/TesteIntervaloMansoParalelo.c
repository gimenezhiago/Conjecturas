#include <stdio.h>
#include <stdbool.h> //Para usar booleano
#include <stdlib.h> //Para usar atoll
#include <omp.h> //Para paralelizar com OpenMP

bool ehPrimo(long long n) {
    if (n <= 1) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    for (long long i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

inline long long somaAlgarismo(long long n) { //inline - troca a funcao por seu conteudo
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

void testeIntervalo(long long fim) {
    const long long LIMITE = 1000000000000; //1 trilhão
    volatile int abortFlag = 0; // sinaliza para encerrar o laço quando um caso falhar
    long long i;

    #pragma omp parallel for schedule(dynamic, 1000) default(none) shared(fim, LIMITE, abortFlag) private(i)
    for (i = 1; i <= fim; i++) {
        int localAbort = 0;
        #pragma omp atomic read
        localAbort = abortFlag;
        if (localAbort) continue; // outro thread já pediu abort

        long long atual = i;
        long long contador = 0;
        bool convergiu = false;

        while (contador < LIMITE) {
            #pragma omp atomic read
            localAbort = abortFlag;
            if (localAbort) break; // verifica sinal de abort

            if (atual == 3 || atual == 7 || atual == 15) {
                convergiu = true;
                break;
            }

            atual = aplicarRegras(atual);
            contador++;

            if (atual <= 0) { // checagem simples para overflow/valor inválido
                #pragma omp critical
                {
                    printf("O numero %lld gerou valor invalido/overflow em %lld iteracoes. ultimo valor = %lld\n", i, contador, atual);
                }
                #pragma omp atomic write
                abortFlag = 1;
                break;
            }
        }

        if (!convergiu) {
            #pragma omp critical
            {
                printf("O numero %lld nao convergiu (>%lld iteracoes). ultimo valor = %lld\n", i, LIMITE, atual);
            }
            #pragma omp atomic write
            abortFlag = 1;
        }

        #pragma omp critical
        {
            if (i % 100000 == 0) {
                printf("Testados %lld numeros\n", i);
            }
        }
    }

    printf("Teste concluido ate %lld\n", fim);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <fim> [num_threads]\n", argv[0]);
        return 1;
    }

    long long fim = atoll(argv[1]);
    if (fim <= 0) {
        printf("Parametro invalido: %s\n", argv[1]);
        return 1;
    }

    // Se passado, define numero de threads OpenMP
    if (argc >= 3) {
        int t = atoi(argv[2]);
        if (t > 0) {
            omp_set_num_threads(t);
            printf("Usando %d threads OpenMP\n", t);
        }
    }

    printf("Testando ate %lld\n", fim);
    testeIntervalo(fim);

    return 0;
}

// Para compilar com GCC (MinGW/MSYS): gcc -O3 -fopenmp TesteIntervaloMansoParalelo.c -o TesteIntervaloMansoParalelo
// Para rodar: ./TesteIntervaloMansoParalelo 100
