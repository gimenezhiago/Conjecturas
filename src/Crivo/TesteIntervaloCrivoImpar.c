#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <sys/resource.h>

#define BLOCO 1048576LL

static long long get_mem_kb(void) {
    struct rusage ru;
    getrusage(RUSAGE_SELF, &ru);
#ifdef __APPLE__
    return ru.ru_maxrss / 1024;
#else
    return ru.ru_maxrss;
#endif
}

void testeCrivo(long long N) {
    if (N < 2) { printf("Nao ha primos.\n"); return; }

    long long limite = (long long)sqrt((double)N) + 1;
    long long tam_pequeno = (limite / 2) + 2;

    bool *pequeno = calloc(tam_pequeno, sizeof(bool));
    if (!pequeno) { fprintf(stderr, "Erro de alocacao.\n"); return; }

    long long total_primos = 1;
    long long total_marcacoes = 0;

    for (long long i = 1; 2*i+1 <= limite; i++) {
        if (!pequeno[i]) {
            long long p = 2*i + 1;
            for (long long j = (p*p - 1)/2; j < tam_pequeno; j += p) {
                if (!pequeno[j]) { pequeno[j] = true; total_marcacoes++; }
            }
        }
    }

    long long n_pp = 0;
    for (long long i = 1; i < tam_pequeno; i++)
        if (!pequeno[i]) n_pp++;

    long long *primos_p = malloc(n_pp * sizeof(long long));
    if (!primos_p) { free(pequeno); fprintf(stderr, "Erro de alocacao.\n"); return; }

    long long idx = 0;
    for (long long i = 1; i < tam_pequeno; i++)
        if (!pequeno[i]) primos_p[idx++] = 2*i + 1;
    free(pequeno);

    for (long long i = 0; i < n_pp; i++)
        if (primos_p[i] <= N) total_primos++;

    bool *bloco = malloc(BLOCO * sizeof(bool));
    if (!bloco) { free(primos_p); fprintf(stderr, "Erro de alocacao.\n"); return; }

    long long base = limite + 1;
    if (base % 2 == 0) base++;

    while (base <= N) {
        long long topo = base + 2*BLOCO - 2;
        if (topo > N) topo = N;
        if (topo % 2 == 0) topo--;

        long long tam_seg = (topo - base) / 2 + 1;
        for (long long k = 0; k < tam_seg; k++) bloco[k] = false;

        for (long long pi = 0; pi < n_pp; pi++) {
            long long p = primos_p[pi];
            long long primeiro = ((base + p - 1) / p) * p;
            if (primeiro % 2 == 0) primeiro += p;
            if (primeiro < p*p) primeiro = p*p;

            for (long long mult = primeiro; mult <= topo; mult += 2*p) {
                long long k = (mult - base) / 2;
                if (!bloco[k]) { bloco[k] = true; total_marcacoes++; }
            }
        }

        for (long long k = 0; k < tam_seg; k++)
            if (!bloco[k]) total_primos++;

        base = topo + 2;
    }

    long long mem_kb = get_mem_kb();

    printf("PRIMOS=%lld\n", total_primos);
    printf("MARCACOES=%lld\n", total_marcacoes);
    printf("MEM_KB=%lld\n", mem_kb);

    free(bloco);
    free(primos_p);
}

int main(int argc, char *argv[]) {
    if (argc < 2) { fprintf(stderr, "Uso: %s <N>\n", argv[0]); return 1; }
    long long N = atoll(argv[1]);
    testeCrivo(N);
    return 0;
}

// Compilar: gcc -O3 TesteIntervaloCrivoImpar.c -o TesteIntervaloCrivoImpar -lm
// Rodar:    ./TesteIntervaloCrivoImpar 100000000000
