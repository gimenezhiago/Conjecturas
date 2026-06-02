#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <sys/resource.h>
#include <omp.h>

#define SET_BIT(arr, i)  ((arr)[(i)>>3] |=  (1u << ((i)&7)))
#define GET_BIT(arr, i)  ((arr)[(i)>>3] &   (1u << ((i)&7)))

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
    long long bytes_pequeno = (tam_pequeno / 8) + 2;

    uint8_t *pequeno = calloc(bytes_pequeno, 1);
    if (!pequeno) { fprintf(stderr, "Erro de alocacao.\n"); return; }

    long long total_primos = 1;
    long long total_marcacoes = 0;

    /* Fase pequena: sequencial */
    for (long long i = 1; 2*i+1 <= limite; i++) {
        if (!GET_BIT(pequeno, i)) {
            long long p = 2*i + 1;
            for (long long j = (p*p-1)/2; j < tam_pequeno; j += p) {
                if (!GET_BIT(pequeno, j)) { SET_BIT(pequeno, j); total_marcacoes++; }
            }
        }
    }

    long long n_pp = 0;
    for (long long i = 1; i < tam_pequeno; i++)
        if (!GET_BIT(pequeno, i)) n_pp++;

    long long *primos_p = malloc(n_pp * sizeof(long long));
    if (!primos_p) { free(pequeno); fprintf(stderr, "Erro de alocacao.\n"); return; }

    long long idx = 0;
    for (long long i = 1; i < tam_pequeno; i++)
        if (!GET_BIT(pequeno, i)) primos_p[idx++] = 2*i + 1;
    free(pequeno);

    for (long long i = 0; i < n_pp; i++)
        if (primos_p[i] <= N) total_primos++;

    /* Fase segmentada: paralela com OpenMP — cada thread tem buffer próprio */
    long long bytes_bloco = (BLOCO / 8) + 2;

    long long base_global = limite + 1;
    if (base_global % 2 == 0) base_global++;

    long long n_segmentos = 0;
    {
        long long b = base_global;
        while (b <= N) {
            n_segmentos++;
            long long topo = b + 2*BLOCO - 2;
            if (topo > N) topo = N;
            if (topo % 2 == 0) topo--;
            b = topo + 2;
        }
    }

    #pragma omp parallel reduction(+:total_marcacoes, total_primos)
    {
        uint8_t *bloco_local = malloc(bytes_bloco);
        if (!bloco_local) {
            #pragma omp cancel parallel
        }

        #pragma omp for schedule(dynamic, 1)
        for (long long seg = 0; seg < n_segmentos; seg++) {
            long long base = base_global + seg * 2 * BLOCO;
            if (base % 2 == 0) base++;

            long long topo = base + 2*BLOCO - 2;
            if (topo > N) topo = N;
            if (topo % 2 == 0) topo--;

            if (base > N) continue;

            long long tam_seg   = (topo - base) / 2 + 1;
            long long bytes_seg = (tam_seg / 8) + 2;

            for (long long k = 0; k < bytes_seg; k++) bloco_local[k] = 0;

            for (long long pi = 0; pi < n_pp; pi++) {
                long long p = primos_p[pi];
                long long primeiro = ((base + p - 1) / p) * p;
                if (primeiro % 2 == 0) primeiro += p;
                if (primeiro < p*p) primeiro = p*p;

                for (long long mult = primeiro; mult <= topo; mult += 2*p) {
                    long long k = (mult - base) / 2;
                    if (!GET_BIT(bloco_local, k)) {
                        SET_BIT(bloco_local, k);
                        total_marcacoes++;
                    }
                }
            }

            for (long long k = 0; k < tam_seg; k++)
                if (!GET_BIT(bloco_local, k)) total_primos++;
        }

        free(bloco_local);
    }

    long long mem_kb = get_mem_kb();

    printf("PRIMOS=%lld\n", total_primos);
    printf("MARCACOES=%lld\n", total_marcacoes);
    printf("MEM_KB=%lld\n", mem_kb);

    free(primos_p);
}

int main(int argc, char *argv[]) {
    if (argc < 2) { fprintf(stderr, "Uso: %s <N>\n", argv[0]); return 1; }
    long long N = atoll(argv[1]);
    testeCrivo(N);
    return 0;
}

// Compilar: gcc -O3 -fopenmp TesteIntervaloCrivoImparBITSET.c -o TesteIntervaloCrivoImparBITSET -lm
// Rodar:    OMP_NUM_THREADS=8 ./TesteIntervaloCrivoImparBITSET 100000000000
