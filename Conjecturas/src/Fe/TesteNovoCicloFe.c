#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

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

long long detectarCiclo(long long n, long long *tamanho_sequencia) {
    // Algoritmo de Floyd para detecção de ciclos (lebre e tartaruga)
    long long lento = aplicarRegras(n);
    long long rapido = aplicarRegras(aplicarRegras(n));
    
    *tamanho_sequencia = 1;

    while (lento != rapido) {
        if (lento == 1 || rapido == 1) {
            return 1; // Convergiu para 1 (ciclo oficial)
        }
        
        if (lento < 0 || rapido < 0) {
            return -1; // Indica overflow ou valor inválido
        }
        
        lento = aplicarRegras(lento);
        rapido = aplicarRegras(aplicarRegras(rapido));
        (*tamanho_sequencia)++;
        
        if (*tamanho_sequencia > 10000) {
            return lento; // Retorna o valor onde o ciclo foi detectado
        }
    }
    
    return lento; // Retorna o valor onde o ciclo foi detectado
}

void imprimirCiclo(long long inicio) {
    long long atual = inicio;
    printf("Ciclo detectado: ");
    int contador = 0;
    
    do {
        printf("%lld ", atual);
        atual = aplicarRegras(atual);
        contador++;
        
        if (contador > 100) {
            printf("... (ciclo muito longo)");
            break;
        }
    } while (atual != inicio);
    
    printf("\n");
}

void buscarCiclos(long long testes) {
    const long long cicloOficial = 1;
    long long ciclosEncontrados = 0;
    
    srand(time(NULL)); // Inicializa o gerador de números aleatórios

    printf("Buscando ciclos em %lld testes aleatorios...\n", testes);
    printf("============================================\n");

    for (long long i = 1; i <= testes; i++) {
        // Gera número aleatório grande
        long long n = ((long long)rand() * rand()) % 9000000000000000000LL + 1000000000000000000LL;
        
        long long tamanho_sequencia = 0;
        long long encontro = detectarCiclo(n, &tamanho_sequencia);

        if (encontro == -1) {
            printf("O numero %lld gerou valor invalido/overflow durante a detecao de ciclo.\n", n);
            continue;
        }

        if (encontro != cicloOficial) {
            printf("\n=== CICLO NOVO ENCONTRADO ===\n");
            printf("Numero inicial: %lld\n", n);
            printf("Valor do ciclo: %lld\n", encontro);
            printf("Tamanho da sequencia ate o ciclo: %lld\n", tamanho_sequencia);
            imprimirCiclo(encontro);
            printf("=============================\n\n");
            ciclosEncontrados++;
        }

        if (i % 10000 == 0) {
            printf("Testes realizados: %lld (Ciclos novos encontrados: %lld)\n", i, ciclosEncontrados);
        }
    }

    if (ciclosEncontrados == 0) {
        printf("\nNenhum ciclo desconhecido foi detectado em %lld testes.\n", testes);
        printf("Todos os numeros testados convergem para 1.\n");
    } else {
        printf("\nTotal de ciclos novos encontrados: %lld\n", ciclosEncontrados);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <numero_de_testes>\n", argv[0]);
        return 1;
    }

    long long testes = atoll(argv[1]);
    
    if (testes <= 0) {
        printf("Parametro invalido: %s\n", argv[1]);
        return 1;
    }

    buscarCiclos(testes);
    
    return 0;
}

// Para compilar: gcc -O3 TesteNovoCicloFe.c -o TesteNovoCicloFe
// Para rodar: ./TesteNovoCicloFe 100000