#include <stdio.h>
#include <cuda_runtime.h>
#include <stdlib.h>
#include <limits.h>

#define LIMITE 10000000LL // 10 milhões de iterações

#define CUDA_CHECK(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            fprintf(stderr, "CUDA error in %s (%s:%d): %s\n", #call, __FILE__, __LINE__, cudaGetErrorString(err)); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

// Regras de Collatz na GPU
__device__ long long aplicarRegras(long long n) {
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

// Verifica se é potência de 2 usando operação bitwise
__device__ bool ehPotenciaDe2(long long n) {
    return (n > 0) && ((n & (n - 1)) == 0);
}

// Kernel CUDA para testar convergência
__global__ void testeConvergenciaKernel(long long inicio, long long fim, 
                                         long long *seq, int tamanhoSeq,
                                         int *resultados, int *indices) {
    long long idx = blockIdx.x * blockDim.x + threadIdx.x;
    long long total = fim - inicio + 1;
    if (idx >= total) return; // evita threads extras

    long long numero = inicio + idx;
    long long local_idx = idx; // índice relativo ao lote

    // Ignora potências de 2
    if (ehPotenciaDe2(numero)) {
        resultados[local_idx] = 1;
        indices[local_idx] = -1; // Marca como potência de 2
        return;
    }

    long long atual = numero;
    long long contador = 0;
    bool convergiu = false;
    int indiceConvergencia = -2;

    while (contador < LIMITE) {
        // Verifica se convergiu para algum valor da sequência
        for (int i = 0; i <= tamanhoSeq; i++) {
            if (atual == seq[i]) {
                convergiu = true;
                indiceConvergencia = i;
                break;
            }
        }

        if (convergiu) break;

        atual = aplicarRegras(atual);
        contador++;

        // aplicarRegras retorna -1 em caso de overflow detectado
        if (atual < 0) {
            printf("GPU: Numero %lld gerou overflow em %lld iteracoes!\n", numero, contador);
            resultados[local_idx] = -1;
            indices[local_idx] = -2;
            return;
        }
    }

    if (!convergiu) {
        printf("GPU: Numero %lld nao convergiu em %lld iteracoes. Ultimo valor: %lld\n", 
               numero, LIMITE, atual);
        resultados[local_idx] = 0;
        indices[local_idx] = -2;
    } else {
        resultados[local_idx] = 1;
        indices[local_idx] = indiceConvergencia;
    }
}

// Gera a sequência a(n) = 4*a(n-1) + 1 na CPU
void gerarSequencia(long long *seq, int n) {
    seq[0] = 1;
    printf("a(0) = %lld\n", seq[0]);
    
    for (int i = 1; i <= n; i++) {
        seq[i] = 4 * seq[i-1] + 1;
        printf("a(%d) = %lld\n", i, seq[i]);
    }
    printf("\n");
}

void testarIntervalo(int n) {
    // Aloca sequência na CPU
    long long *h_seq = (long long*)malloc(sizeof(long long) * (n + 1));
    long long *acumulador = (long long*)calloc((n + 1), sizeof(long long));
    
    if (!h_seq || !acumulador) {
        printf("Erro ao alocar memoria na CPU!\n");
        return;
    }
    
    printf("============================================\n");
    printf("GERANDO SEQUENCIA a(n) = 4*a(n-1) + 1\n");
    printf("============================================\n");
    gerarSequencia(h_seq, n);
    
    printf("============================================\n");
    printf("TESTANDO CONVERGENCIA PARA COLLATZ (CUDA)\n");
    printf("============================================\n");
    printf("Verificando se numeros entre a(%d)=%lld e a(%d)=%lld convergem\n\n", 
           n-1, h_seq[n-1], n, h_seq[n]);
    
    // Informações da GPU
    int deviceCount;
    CUDA_CHECK(cudaGetDeviceCount(&deviceCount));
    if (deviceCount == 0) {
        printf("Nenhuma GPU CUDA encontrada.\n");
        free(h_seq);
        free(acumulador);
        return;
    }
    
    cudaDeviceProp prop;
    CUDA_CHECK(cudaGetDeviceProperties(&prop, 0));
    printf("GPU: %s\n", prop.name);
    printf("Compute Capability: %d.%d\n", prop.major, prop.minor);
    printf("Multiprocessors: %d\n\n", prop.multiprocessorCount);
    
    const int threadsPorBloco = 256;
    const long long max_numeros_por_lote = 1000000; // 1 milhão por lote
    
    long long inicio_intervalo = h_seq[n-1] + 1;
    long long fim_intervalo = h_seq[n] - 1;
    long long totalNumeros = fim_intervalo - inicio_intervalo + 1;
    
    printf("Total de numeros a testar: %lld\n", totalNumeros);
    printf("Threads por bloco: %d\n", threadsPorBloco);
    printf("Numeros por lote: %lld\n\n", max_numeros_por_lote);
    
    // Aloca sequência na GPU (copiada uma vez)
    long long *d_seq;
    CUDA_CHECK(cudaMalloc(&d_seq, sizeof(long long) * (n + 1)));
    CUDA_CHECK(cudaMemcpy(d_seq, h_seq, sizeof(long long) * (n + 1), cudaMemcpyHostToDevice));
    
    long long totalTestados = 0;
    long long totalConvergidos = 0;
    long long potenciasDe2 = 0;
    bool encontrouFalha = false;
    
    cudaEvent_t start, stop;
    CUDA_CHECK(cudaEventCreate(&start));
    CUDA_CHECK(cudaEventCreate(&stop));
    CUDA_CHECK(cudaEventRecord(start));
    
    // Processa em lotes
    for (long long lote_inicio = inicio_intervalo; lote_inicio <= fim_intervalo; lote_inicio += max_numeros_por_lote) {
        long long lote_fim = (lote_inicio + max_numeros_por_lote - 1 < fim_intervalo) ? 
                              lote_inicio + max_numeros_por_lote - 1 : fim_intervalo;
        long long numeros_neste_lote = lote_fim - lote_inicio + 1;
        
        int blocos = (int)((numeros_neste_lote + threadsPorBloco - 1) / threadsPorBloco);
        
        // Aloca memória para resultados e índices
        int *d_resultados, *d_indices;
        CUDA_CHECK(cudaMalloc(&d_resultados, numeros_neste_lote * sizeof(int)));
        CUDA_CHECK(cudaMalloc(&d_indices, numeros_neste_lote * sizeof(int)));
        CUDA_CHECK(cudaMemset(d_resultados, 0, numeros_neste_lote * sizeof(int)));

        // Inicializa índices com -2 no host e copia para o device (cudaMemset com valor negativo é byte-wise)
        int *h_init_indices = (int*)malloc(numeros_neste_lote * sizeof(int));
        if (!h_init_indices) { fprintf(stderr, "Erro de alocacao host para indices\n"); exit(EXIT_FAILURE); }
        for (long long ii = 0; ii < numeros_neste_lote; ++ii) h_init_indices[ii] = -2;
        CUDA_CHECK(cudaMemcpy(d_indices, h_init_indices, numeros_neste_lote * sizeof(int), cudaMemcpyHostToDevice));
        free(h_init_indices);
        
        // Executa kernel
        testeConvergenciaKernel<<<blocos, threadsPorBloco>>>(
            lote_inicio, lote_fim, d_seq, n, d_resultados, d_indices
        );
        
        CUDA_CHECK(cudaDeviceSynchronize());
        CUDA_CHECK(cudaGetLastError());
        
        // Copia resultados para CPU
        int *h_resultados, *h_indices;
        CUDA_CHECK(cudaMallocHost((void**)&h_resultados, numeros_neste_lote * sizeof(int)));
        CUDA_CHECK(cudaMallocHost((void**)&h_indices, numeros_neste_lote * sizeof(int)));
        CUDA_CHECK(cudaMemcpy(h_resultados, d_resultados, numeros_neste_lote * sizeof(int), cudaMemcpyDeviceToHost));
        CUDA_CHECK(cudaMemcpy(h_indices, d_indices, numeros_neste_lote * sizeof(int), cudaMemcpyDeviceToHost));
        
        // Processa resultados
        for (long long j = 0; j < numeros_neste_lote; j++) {
            if (h_resultados[j] == 1) {
                totalConvergidos++;
                if (h_indices[j] == -1) {
                    potenciasDe2++;
                } else if (h_indices[j] >= 0) {
                    acumulador[h_indices[j]]++;
                }
            } else {
                long long numero = lote_inicio + j;
                printf("\nFALHA detectada no numero %lld!\n", numero);
                encontrouFalha = true;
                break;
            }
        }
        
        CUDA_CHECK(cudaFreeHost(h_resultados));
        CUDA_CHECK(cudaFreeHost(h_indices));
        CUDA_CHECK(cudaFree(d_resultados));
        CUDA_CHECK(cudaFree(d_indices));
        
        totalTestados += numeros_neste_lote;
        printf("Progresso: %lld numeros testados (lote ate %lld)\n", totalTestados, lote_fim);
        
        if (encontrouFalha) {
            printf("Interrompendo execucao devido a falha.\n");
            break;
        }
    }
    
    CUDA_CHECK(cudaEventRecord(stop));
    CUDA_CHECK(cudaEventSynchronize(stop));
    
    float milliseconds = 0;
    CUDA_CHECK(cudaEventElapsedTime(&milliseconds, start, stop));
    float tempo = milliseconds / 1000.0f;
    
    CUDA_CHECK(cudaFree(d_seq));
    CUDA_CHECK(cudaEventDestroy(start));
    CUDA_CHECK(cudaEventDestroy(stop));
    
    if (!encontrouFalha) {
        printf("\n============================================\n");
        printf("RESULTADOS\n");
        printf("============================================\n");
        printf("OK -- todos entre %lld e %lld convergiram\n", h_seq[n-1], h_seq[n]);
        printf("Total testados: %lld\n", totalTestados);
        printf("Total convergidos: %lld\n", totalConvergidos);
        printf("Potencias de 2 (ignoradas): %lld\n", potenciasDe2);
        printf("Tempo: %.2f segundos\n", tempo);
        printf("Throughput: %.0f numeros/segundo\n\n", totalTestados / tempo);
        
        printf("============================================\n");
        printf("ACUMULADOR - DISTRIBUICAO DE CONVERGENCIA\n");
        printf("============================================\n");
        for (int i = 0; i <= n; i++) {
            printf("a(%d)=%lld, convergiram %lld\n", i, h_seq[i], acumulador[i]);
        }
    }
    
    free(h_seq);
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
    if (n <= 0 ) {
        printf("Parametro invalido. Use n entre 1 e 15\n");
        return 1;
    }
    
    testarIntervalo(n);
    
    return 0;
}

// Para compilar: nvcc -O3 TesteSequenciaCollatzParaleloCuda.cu -o TesteSequenciaCollatzParaleloCuda
// Para rodar: ./TesteSequenciaCollatzParaleloCuda 5
