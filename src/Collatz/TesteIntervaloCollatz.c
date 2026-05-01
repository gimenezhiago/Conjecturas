#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>
#include <limits.h>

// Função para obter tempo de alta resolução
double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// Regras de Collatz (par/ímpar)
long long aplicarRegras(long long n) {
    if (n % 2 == 0) {
        return n / 2;
    } else {
        // Evita overflow em 3*n + 1
        if (n > (LLONG_MAX - 1) / 3) {
            return -1; // sinaliza overflow
        }
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
            printf("Numero %lld gerou overflow!\n", num);
            return false;
        }
    }
    
    printf("FALHA: Numero %lld nao convergiu em %lld iteracoes. Ultimo valor: %lld\n", 
           num, LIMITE, atual);
    return false;
}

void testarIntervalo(int n) {
    long long *seq = malloc(sizeof(long long) * (n + 1));
    long long *acumulador = calloc((n + 1), sizeof(long long));
    
    if (!seq || !acumulador) {
        printf("Erro ao alocar memoria!\n");
        return;
    }
    
    printf("============================================\n");
    printf("GERANDO SEQUENCIA a(n) = 4*a(n-1) + 1\n");
    printf("============================================\n");
    gerarSequencia(seq, n);
    
    printf("============================================\n");
    printf("TESTANDO CONVERGENCIA PARA COLLATZ\n");
    printf("============================================\n");
    printf("Verificando se numeros entre a(%d)=%lld e a(%d)=%lld convergem\n\n", 
           n-1, seq[n-1], n, seq[n]);
    
    double inicio = get_time();
    long long totalTestados = 0;
    long long totalConvergidos = 0;
    long long potenciasDe2 = 0;
    
    // Testa todos os números entre seq[n-1]+1 e seq[n]-1
    for (long long num = seq[n-1] + 1; num < seq[n]; num++) {
        int indiceConvergencia = -1;
        
        if (verificarConvergencia(num, seq, n, &indiceConvergencia)) {
            if (indiceConvergencia == -1) {
                potenciasDe2++;
            } else {
                acumulador[indiceConvergencia]++;
            }
            totalConvergidos++;
        } else {
            printf("\nTeste INTERROMPIDO!\n");
            free(seq);
            free(acumulador);
            return;
        }
        
        totalTestados++;
        
        // Progresso a cada 10.000 números
        if (totalTestados % 10000 == 0) {
            printf("Progresso: %lld numeros testados...\n", totalTestados);
        }
    }
    
    double fim = get_time();
    double tempo = fim - inicio;
    
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
        printf("Uso: %s <n>\n", argv[0]);
        printf("Testa se todos os numeros entre a(n-1) e a(n) convergem para Collatz\n");
        printf("onde a(i) = 4*a(i-1) + 1, com a(0) = 1\n");
        return 1;
    }
    
    int n = atoi(argv[1]);
    if (n <= 0) {
        printf("Parametro invalido\n");
        return 1;
    }
    
    testarIntervalo(n);
    
    return 0;
}

// Para compilar: gcc -O3 TesteSequenciaCollatz.c -o TesteSequenciaCollatz
// Para rodar: ./TesteSequenciaCollatz 5
