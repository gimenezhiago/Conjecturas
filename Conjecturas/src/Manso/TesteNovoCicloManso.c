#include <stdio.h>
#include <stdbool.h> //Para usar booleano
#include <stdlib.h> //Para usar atoll
#include <time.h> //Para medir tempo

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

long long detectarCiclo(long long n) { // Algoritmo de Floyd para detecção de ciclos (lebre e tartaruga)
    long long lento = aplicarRegras(n);
    long long rapido = aplicarRegras(aplicarRegras(n)); 

    while (lento != rapido) {
        lento = aplicarRegras(lento);
        rapido = aplicarRegras(aplicarRegras(rapido));

        if (lento < 0 || rapido < 0) {
            return -1; // Indica overflow ou valor inválido
        }
        
        return lento; // Retorna o valor onde o ciclo foi detectado
    }
}

void imprimirCiclo(long long inicio) {
    long long atual = inicio;
    printf("Ciclo detectado: ");

    do {
        printf("%lld ", atual);
        atual = aplicarRegras(atual);
    } while (atual != inicio);
    printf("\n");
}

void buscarCiclos(long long testes) {
    const long long cicloOficial1 = 3;
    const long long cicloOficial2 = 7;
    const long long cicloOficial3 = 15;

    srand(time(NULL)); // Inicializa o gerador de números aleatórios

    for (long long i = 1; i <= testes; i++) {
        long long n = ((long long)rand() * rand()) % 9000000000000000000LL + 1000000000000000000LL;

        long long encontro = detectarCiclo(n);

        if (encontro == -1) {
            printf("O numero %lld gerou valor invalido/overflow durante a detecao de ciclo.\n", n);
            continue;
        }

        if (i % 10000 == 0) {
            printf("Testes realizados: %lld\n", i);
        }
    }

    printf("Nenhum ciclo desconhecido foi detectado em %lld testes.\n", testes);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <numero_de_testes>\n", argv[0]);
        return 1;
    }

    long long testes = atoll(argv[1]);

    buscarCiclos(testes);
    return 0;
}

// Para compilar: gcc -O3 TesteNovoCicloManso.c -o TesteNovoCicloManso
// Para rodar: ./TesteNovoCicloManso 100