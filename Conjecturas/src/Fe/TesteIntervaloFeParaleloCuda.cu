#include <stdio.h>
#include <cuda_runtime.h>
#include <stdlib.h>

#define LIMITE 1000000000000LL //1 trilhão

__device__ long long somaDigitos(long long n) {
    long long soma = 0;
    for (; n > 0; n /= 10) {
        soma += n % 10;
    }
    return soma;
}

__device__ long long aplicarRegras(long long n) {
    if (n % 2 == 0) return n / 2;
    if (n % 3 == 0) return n / 3;
    if (n % 5 == 0) return n / 5;
    if (n % 7 == 0) return n / 7;

    return n + somaDigitos(n);
}

__global__ void testeConvergenciaKernel(long long inicio, long long fim, int *resultados) {
    long long idx = blockIdx.x * blockDim.x + threadIdx.x;
    long long numero = inicio + idx;

    if (numero > fim) return;

    long long local_idx = numero - inicio;

    long long atual = numero;
    long long contador = 0;
    bool convergiu = false;

    while (contador < LIMITE) {
        if (atual == 1) {
            convergiu = true;
            break;
        }

        atual = aplicarRegras(atual);
        contador++;

        if (atual <= 0) {
            printf("O numero %lld gerou valor invalido/overflow em %lld iteracoes. ultimo valor = %lld\n", 
                   numero, contador, atual);
            resultados[local_idx] = -1;
            return;
        }
    }

    if (!convergiu) {
        printf("O numero %lld nao convergiu (>%lld iteracoes). ultimo valor = %lld\n", 
                   numero, contador, atual);
        resultados[local_idx] = 0;
    } else {
        resultados[local_idx] = 1;
    }
}

#define CUDA_CHECK(call) \
    do { \
        cudaError_t err = call; \
        if (err != cudaSuccess) { \
            fprintf(stderr, "CUDA error in %s (%s:%d): %s\n", #call, __FILE__, __LINE__, cudaGetErrorString(err)); \
            exit(EXIT_FAILURE); \
        } \
    } while (0)

void testeIntervalo(long long fim) {
    const int threadsPorBloco = 256;
    const long long max_numeros_por_lote = 1000000; // 1 milhão (ajuste conforme memória/GPU)

    printf("Threads por bloco: %d\n", threadsPorBloco);

    for (long long lote_inicio = 1; lote_inicio <= fim; lote_inicio += max_numeros_por_lote) {
        long long lote_fim = (lote_inicio + max_numeros_por_lote - 1 < fim) ? 
                              lote_inicio + max_numeros_por_lote - 1 : fim;
        long long numeros_neste_lote = lote_fim - lote_inicio + 1;
        
        int blocos = (int)((numeros_neste_lote + threadsPorBloco - 1) / threadsPorBloco);
        
        int *d_resultados = NULL;
        CUDA_CHECK(cudaMalloc(&d_resultados, numeros_neste_lote * sizeof(int)));
        /* Inicializa com 1 (assumimos convergência) para facilitar verificações no host */
        CUDA_CHECK(cudaMemset(d_resultados, 1, numeros_neste_lote * sizeof(int)));

        testeConvergenciaKernel<<<blocos, threadsPorBloco>>>(lote_inicio, lote_fim, d_resultados);

        CUDA_CHECK(cudaDeviceSynchronize());
        CUDA_CHECK(cudaGetLastError());

        /* Copia resultados para host (usa memória pinada para maior banda) */
        int *h_resultados = NULL;
        CUDA_CHECK(cudaMallocHost((void**)&h_resultados, numeros_neste_lote * sizeof(int)));
        CUDA_CHECK(cudaMemcpy(h_resultados, d_resultados, numeros_neste_lote * sizeof(int), cudaMemcpyDeviceToHost));

        /* Varre resultados e interrompe se encontrar falha (overflow ou não convergiu) */
        bool encontrouFalha = false;
        for (long long j = 0; j < numeros_neste_lote; ++j) {
            if (h_resultados[j] != 1) {
                long long numero = lote_inicio + j;
                if (h_resultados[j] == -1) {
                    printf("O numero %lld gerou valor invalido/overflow (detectado no host).\n", numero);
                } else if (h_resultados[j] == 0) {
                    printf("O numero %lld nao convergiu (>%lld iteracoes).\n", numero, LIMITE);
                } else {
                    printf("O numero %lld retornou codigo inesperado %d\n", numero, h_resultados[j]);
                }
                encontrouFalha = true;
                break;
            }
        }

        CUDA_CHECK(cudaFreeHost(h_resultados));
        CUDA_CHECK(cudaFree(d_resultados));

        printf("Testados %lld numeros (lote ate %lld)\n", numeros_neste_lote, lote_fim);

        if (encontrouFalha) {
            printf("Interrompendo execucao devido a falha detectada.\n");
            return; // encerra o teste antecipadamente
        }
    }

    printf("Teste de intervalo concluido ate %lld\n", fim);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <fim_do_intervalo>\n", argv[0]);
        return 1;
    }

    long long fim = atoll(argv[1]);
    if (fim <= 0) {
        printf("Parametro invalido: %s\n", argv[1]);
        return 1;
    }

    int deviceCount;
    CUDA_CHECK(cudaGetDeviceCount(&deviceCount));
    if (deviceCount == 0) {
        printf("Nenhuma GPU CUDA encontrado.\n");
        return 1;
    }

    printf("Testando ate %lld\n", fim);
    testeIntervalo(fim);
    return 0;
}

// Para compilar: nvcc -O3 TesteIntervaloFeParaleloCuda.cu -o TesteIntervaloFeParaleloCuda
// Para rodar: ./TesteIntervaloFeParaleloCuda 100
