#include <stdio.h>
#include <cuda_runtime.h>

#define LIMITE 1000000000000LL //1 trilhão

__device__ bool ehPrimo(long long int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (long long int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

__device__ long long somaAlgarismo(long long n) {
    long long soma = 0;
    for (; n > 0; n /= 10) {
        soma += n % 10;
    }
}

__device__ long long aplicarRegras(long long n) {
    if (ehPrimo(n)) {
        return n + (n + 1);
    } else if (n % 5 == 0) {
        return n / 5;
    } else if (n % 9 == 0) {
        return n / 9;
    } else if (n % 2 == 0) {
        return n / 2;
    } else {
        return somaAlgarismo(n);
    }
}

__global__ void testeConvergenciaKernel(long long inicio, long long fim, int *resultados) {
    long long idx = blockIdx.x * blockDim.x + threadIdx.x;
    long long numero = inicio + idx;

    if (numero > fim) return;

    long long atual = numero;
    long long contador = 0;
    bool convergiu = false;

    while (contador < LIMITE) {
        if (atual == 3 || atual == 7 || atual == 15) {
            convergiu = true;
            break;
        }

        atual = aplicarRegras(atual);
        contador++;

        if (atual <= 0) {
            printf("O numero %lld gerou valor invalido/overflow em %lld iteracoes. ultimo valor = %lld\n", 
                   numero, contador, atual);
            resultados[idx] = -1;
            return;
        }
    }

    if (!convergiu) {
        printf("O numero %lld gerou valor invalido/overflow em %lld iteracoes. ultimo valor = %lld\n", 
                   numero, contador, atual);
        resultados[idx] = 0;
    } else {
        resultados[idx] = 1;
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
    const long long max_numeros_por_lote = 1000000; // 1 milhão

    printf("Threads por bloco: %d\n", threadsPorBloco);

     for (long long lote_inicio = 1; lote_inicio <= fim; lote_inicio += MAX_NUMEROS_POR_LOTE) {
        long long lote_fim = (lote_inicio + MAX_NUMEROS_POR_LOTE - 1 < fim) ? 
                              lote_inicio + MAX_NUMEROS_POR_LOTE - 1 : fim;
        long long numeros_neste_lote = lote_fim - lote_inicio + 1;
        
        int blocos = (numeros_neste_lote + THREADS_POR_BLOCO - 1) / THREADS_POR_BLOCO;
        
        int *d_resultados;
        CUDA_CHECK(cudaMalloc(&d_resultados, numeros_neste_lote * sizeof(int)));
        
        testeConvergenciaKernel<<<blocos, THREADS_POR_BLOCO>>>(lote_inicio, lote_fim, d_resultados);
        
        CUDA_CHECK(cudaDeviceSynchronize());
        CUDA_CHECK(cudaGetLastError());
        
        CUDA_CHECK(cudaFree(d_resultados));
        
        printf("Testados %lld numeros (lote ate %lld)\n", lote_fim, lote_fim);
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

    cudaDeviceProp prop;
    CUDA_CHECK(cudaGetDeviceProperties(&prop, 0));
    printf("Usando GPU: %s\n", prop.name);
    printf("Memoria Global: %lu MB\n", prop.totalGlobalMem / (1024 * 1024));
    printf("Compute Capability: %d.%d\n", prop.major, prop.minor);

    printf("Testando ate %lld\n", fim);
    testeIntervalo(fim);
    return 0;
}

// Para compilar: nvcc -O3 TesteIntervaloMansoParalelo.cu -o TesteIntervaloMansoParalelo
// Para rodar: ./TesteIntervaloMansoParalelo 100