#include <stdio.h>
#include <cuda_runtime.h>
#include <stdlib.h>
#include <time.h>

struct Resultado {
    long long total;
    long long primos;
    long long semiprimos;
    long long outros;
};

__device__ bool ehPrimo(long long int n) {
    if (n <= 1) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    for (long long i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

// Retorna o número de fatores primos distintos e verifica se é semiprimo
__device__ bool ehSemiprimo(long long n) {
    if (n <= 1) return false;
    
    int contadorPrimos = 0;
    int contadorTotal = 0;
    long long temp = n;
    
    // Verifica fator 2
    if (temp % 2 == 0) {
        contadorPrimos++;
        while (temp % 2 == 0) {
            contadorTotal++;
            temp /= 2;
        }
    }
    
    // Verifica fatores ímpares
    for (long long i = 3; i * i <= temp; i += 2) {
        if (temp % i == 0) {
            contadorPrimos++;
            while (temp % i == 0) {
                contadorTotal++;
                temp /= i;
            }
        }
    }
    
    // Se sobrou algo maior que 1, é um fator primo
    if (temp > 1) {
        contadorPrimos++;
        contadorTotal++;
    }
    
    // Semiprimo = produto de exatamente 2 primos distintos
    return (contadorPrimos == 2 && contadorTotal == 2);
}

__global__ void testePolinomioKernel(long long inicio, long long fim, int *tipos) {
    long long idx = blockIdx.x * blockDim.x + threadIdx.x;
    long long n = inicio + idx * 3; // n ≡ 1 (mod 3)
    
    if (n > fim) return;
    
    long long local_idx = idx;
    
    // Calcula n^2 + 3n + 3
    long long valor = n * n + 3 * n + 3;
    
    // Verifica overflow
    if (valor <= 0 || valor < n) {
        tipos[local_idx] = -1; // overflow
        return;
    }
    
    // Classifica o número
    if (ehPrimo(valor)) {
        tipos[local_idx] = 0; // primo
    } else if (ehSemiprimo(valor)) {
        tipos[local_idx] = 1; // semiprimo
    } else {
        tipos[local_idx] = 2; // outro
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

void testeIntervalo(long long n_max) {
    const int threadsPorBloco = 256;
    const long long max_numeros_por_lote = 1000000; // 1 milhão
    
    clock_t inicio = clock();
    printf("Threads por bloco: %d\n", threadsPorBloco);
    printf("Testando polinomio n^2 + 3n + 3 para n congruente a 1 mod 3\n");
    printf("Intervalo: n de 1 ate %lld\n\n", n_max);
    
    long long total_primos = 0;
    long long total_semiprimos = 0;
    long long total_outros = 0;
    long long total_testados = 0;
    
    // Calcula quantos números congruentes a 1 mod 3 existem no intervalo
    long long qtd_numeros = (n_max - 1) / 3 + 1;
    
    for (long long lote_inicio = 1; lote_inicio <= n_max; lote_inicio += max_numeros_por_lote * 3) {
        long long lote_fim = (lote_inicio + max_numeros_por_lote * 3 - 1 < n_max) ? 
                              lote_inicio + max_numeros_por_lote * 3 - 1 : n_max;
        
        // Calcula quantos números neste lote (congruentes a 1 mod 3)
        long long numeros_neste_lote = ((lote_fim - lote_inicio) / 3) + 1;
        
        int blocos = (int)((numeros_neste_lote + threadsPorBloco - 1) / threadsPorBloco);
        
        int *d_tipos = NULL;
        CUDA_CHECK(cudaMalloc(&d_tipos, numeros_neste_lote * sizeof(int)));
        CUDA_CHECK(cudaMemset(d_tipos, 0, numeros_neste_lote * sizeof(int)));
        
        testePolinomioKernel<<<blocos, threadsPorBloco>>>(lote_inicio, lote_fim, d_tipos);
        
        CUDA_CHECK(cudaDeviceSynchronize());
        CUDA_CHECK(cudaGetLastError());
        
        int *h_tipos = NULL;
        CUDA_CHECK(cudaMallocHost((void**)&h_tipos, numeros_neste_lote * sizeof(int)));
        CUDA_CHECK(cudaMemcpy(h_tipos, d_tipos, numeros_neste_lote * sizeof(int), cudaMemcpyDeviceToHost));
        
        // Conta resultados
        long long primos_lote = 0;
        long long semiprimos_lote = 0;
        long long outros_lote = 0;
        
        for (long long j = 0; j < numeros_neste_lote; ++j) {
            if (h_tipos[j] == -1) {
                printf("Overflow detectado!\n");
                continue;
            }
            
            if (h_tipos[j] == 0) primos_lote++;
            else if (h_tipos[j] == 1) semiprimos_lote++;
            else if (h_tipos[j] == 2) outros_lote++;
        }
        
        total_primos += primos_lote;
        total_semiprimos += semiprimos_lote;
        total_outros += outros_lote;
        total_testados += numeros_neste_lote;
        
        CUDA_CHECK(cudaFreeHost(h_tipos));
        CUDA_CHECK(cudaFree(d_tipos));
        
        // Exibe progresso a cada lote
        double perc_primos = (total_testados > 0) ? (100.0 * total_primos / total_testados) : 0;
        double perc_semiprimos = (total_testados > 0) ? (100.0 * total_semiprimos / total_testados) : 0;
        double perc_outros = (total_testados > 0) ? (100.0 * total_outros / total_testados) : 0;
        
        printf("Progresso: n ate %lld | Testados: %lld | Primos: %.2f%% | Semiprimos: %.2f%% | Outros: %.2f%%\n",
               lote_inicio + (numeros_neste_lote - 1) * 3, total_testados, 
               perc_primos, perc_semiprimos, perc_outros);
    }
    
    clock_t fim_tempo = clock();
    double tempo_decorrido = (double)(fim_tempo - inicio) / CLOCKS_PER_SEC;
    
    printf("\n========== RESULTADO FINAL ==========\n");
    printf("Total de numeros testados: %lld\n", total_testados);
    printf("Numeros primos: %lld (%.2f%%)\n", total_primos, 
           100.0 * total_primos / total_testados);
    printf("Numeros semiprimos: %lld (%.2f%%)\n", total_semiprimos, 
           100.0 * total_semiprimos / total_testados);
    printf("Outros numeros: %lld (%.2f%%)\n", total_outros, 
           100.0 * total_outros / total_testados);
    printf("\nTempo total: %.2f segundos\n", tempo_decorrido);
    printf("Throughput: %.0f numeros/segundo\n", total_testados / tempo_decorrido);
    printf("====================================\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <n_maximo>\n", argv[0]);
        return 1;
    }
    
    long long n_max = atoll(argv[1]);
    if (n_max <= 0) {
        printf("Parametro invalido: %s\n", argv[1]);
        return 1;
    }
    
    // Ajusta para o maior número congruente a 1 mod 3
    if (n_max % 3 != 1) {
        n_max = n_max - (n_max % 3) + 1;
        if (n_max > atoll(argv[1])) n_max -= 3;
    }
    
    int deviceCount;
    CUDA_CHECK(cudaGetDeviceCount(&deviceCount));
    if (deviceCount == 0) {
        printf("Nenhuma GPU CUDA encontrada.\n");
        return 1;
    }
    
    cudaDeviceProp prop;
    CUDA_CHECK(cudaGetDeviceProperties(&prop, 0));
    printf("GPU: %s\n", prop.name);
    printf("Compute Capability: %d.%d\n\n", prop.major, prop.minor);
    
    testeIntervalo(n_max);
    return 0;
}

// Para compilar: nvcc -O3 TestePolinomioConjectura.cu -o TestePolinomioConjectura
// Para rodar: ./TestePolinomioConjectura 1000000
