#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

// Regras de Collatz (par/ímpar)
long long aplicarRegras(long long n) {
    if (n % 2 == 0) {
        return n / 2;
    } else {
        return 3 * n + 1;
    }
}

// Gera a sequência a(n) = 4*a(n-1) + 1
void gerarSequencia(long long *seq, int n) {
    seq[0] = 1;
    printf("a(0) = %lld\n", seq[0]);
    
    for (int i = 1; i <= n; i++) {
        seq[i] = 4 * seq[i-1] + 1;
        printf("a(%d) = %lld\n", i, seq[i]);
    }
    printf("\n");
}

// Verifica se um número converge para algum valor da sequência
bool verificarConvergencia(long long num, long long *seq, int tamanhoSeq, int *indiceDaSequencia) {
    const long long LIMITE = 10000000; // 10 milhões de iterações
    long long atual = num;
    long long contador = 0;
    
    // Ignora potências de 2 (otimização do código original)
    bool ehPotenciaDe2 = (num > 0) && ((num & (num - 1)) == 0);
    if (ehPotenciaDe2) {
        *indiceDaSequencia = -1; // Marca como potência de 2
        return true;
    }
    
    while (contador < LIMITE) {
        // Verifica se convergiu para algum valor da sequência
        for (int i = 0; i <= tamanhoSeq; i++) {
            if (atual == seq[i]) {
                *indiceDaSequencia = i;
                return true;
            }
        }
        
        atual = aplicarRegras(atual);
        contador++;
        
        if (atual <= 0) {
            return false;
        }
    }
    
    return false;
}

void testarIntervalo(int n) {
    long long *seq = malloc(sizeof(long long) * (n + 1));
    long long *acumulador = calloc(sizeof(long long), n + 1);
    
    if (!seq || !acumulador) {
        printf("Erro ao alocar memoria!\n");
        return;
    }
    
    printf("============================================\n");
    printf("GERANDO SEQUENCIA a(n) = 4*a(n-1) + 1\n");
    printf("============================================\n");
    gerarSequencia(seq, n);
    
    printf("============================================\n");
    printf("TESTANDO CONVERGENCIA PARA COLLATZ (PARALELO)\n");
    printf("============================================\n");
    printf("Verificando se numeros entre a(%d)=%lld e a(%d)=%lld convergem\n", 
           n-1, seq[n-1], n, seq[n]);
    
    int numThreads = omp_get_max_threads();
    printf("Threads disponiveis: %d\n\n", numThreads);
    
    double inicio = omp_get_wtime();
    long long totalTestados = 0;
    long long totalConvergidos = 0;
    long long potenciasDe2 = 0;
    volatile int abortFlag = 0;
    
    long long intervalo = seq[n] - seq[n-1] - 1;
    long long num;
    
    // Paralelização com OpenMP
    #pragma omp parallel for schedule(dynamic, 1000) default(none) \
        shared(seq, n, intervalo, acumulador, abortFlag, potenciasDe2, totalConvergidos, totalTestados) \
        private(num)
    for (long long i = 1; i <= intervalo; i++) {
        int localAbort = 0;
        #pragma omp atomic read
        localAbort = abortFlag;
        
        if (localAbort) continue;
        
        num = seq[n-1] + i;
        int indiceConvergencia = -1;
        
        if (verificarConvergencia(num, seq, n, &indiceConvergencia)) {
            if (indiceConvergencia == -1) {
                #pragma omp atomic
                potenciasDe2++;
            } else {
                #pragma omp atomic
                acumulador[indiceConvergencia]++;
            }
            
            #pragma omp atomic
            totalConvergidos++;
        } else {
            #pragma omp critical
            {
                printf("FALHA: Numero %lld nao convergiu!\n", num);
            }
            #pragma omp atomic write
            abortFlag = 1;
        }
        
        #pragma omp atomic
        totalTestados++;
        
        // Progresso
        long long local_testados;
        #pragma omp atomic read
        local_testados = totalTestados;
        
        if (local_testados % 100000 == 0) {
            #pragma omp critical
            {
                printf("Progresso: %lld numeros testados...\n", local_testados);
            }
        }
    }
    
    double fim = omp_get_wtime();
    double tempo = fim - inicio;
    
    if (abortFlag) {
        printf("\nTeste INTERROMPIDO devido a falha!\n");
        free(seq);
        free(acumulador);
        return;
    }
    
    printf("\n============================================\n");
    printf("RESULTADOS\n");
    printf("============================================\n");
    printf("OK -- todos entre %lld e %lld convergiram\n", seq[n-1], seq[n]);
    printf("Total testados: %lld\n", totalTestados);
    printf("Total convergidos: %lld\n", totalConvergidos);
    printf("Potencias de 2 (ignoradas): %lld\n", potenciasDe2);
    printf("Tempo: %.2f segundos\n", tempo);
    printf("Throughput: %.0f numeros/segundo\n\n", totalTestados / tempo);
    
    printf("============================================\n");
    printf("ACUMULADOR - DISTRIBUICAO DE CONVERGENCIA\n");
    printf("============================================\n");
    for (int i = 0; i <= n; i++) {
        printf("a(%d)=%lld, convergiram %lld\n", i, seq[i], acumulador[i]);
    }
    
    free(seq);
    free(acumulador);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <n> [num_threads]\n", argv[0]);
        printf("Testa se todos os numeros entre a(n-1) e a(n) convergem para Collatz\n");
        printf("onde a(i) = 4*a(i-1) + 1, com a(0) = 1\n");
        return 1;
    }
    
    int n = atoi(argv[1]);
    if (n <= 0 || n > 15) {
        printf("Parametro invalido. Use n entre 1 e 15\n");
        return 1;
    }
    
    testarIntervalo(n);
    
    return 0;
}

// Para compilar: gcc -O3 -fopenmp TesteSequenciaCollatzParalelo.c -o TesteSequenciaCollatzParalelo
// Para rodar: ./TesteSequenciaCollatzParalelo 5
// Com threads específicas: ./TesteSequenciaCollatzParalelo 5 8