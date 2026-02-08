#include <stdio.h>
#include <cuda_runtime.h>
#include <stdlib.h>
#include <time.h>

#define LIMITE_ITERACOES 1000000000LL // 1 bilhão de iterações como limite

__device__ long long somaDigitos(long long n) {
    long long soma = 0;
    while (n > 0) {
        soma += n % 10;
        n /= 10;
    }
    return soma;
}

__device__ bool ehPotenciaDe10(long long n) {
    if (n < 1) return false;
    while (n > 1) {
        if (n % 10 != 0) return false;
        n /= 10;
    }
    return true;
}

__device__ long long testarConvergencia(long long n, long long *num_operacoes) {
    long long atual = n;
    long long operacoes = 0;
    
    while (atual != 1 && operacoes < LIMITE_ITERACOES) {
        // Se for potência de 10, divide por si mesmo
        if (ehPotenciaDe10(atual)) {
            atual = atual / atual; // sempre resulta em 1
            operacoes++;
            break;
        }
        
        long long soma = somaDigitos(atual);
        
        if (soma == 0) {
            // Caso impossível para n > 0, mas verificamos por segurança
            *num_operacoes = -1;
            return atual;
        }
        
        // Verifica se a divisão é exata
        if (atual % soma == 0) {
            atual = atual / soma;
        } else {
            atual = atual + soma;
        }
        
        operacoes++;
        
        // Verifica overflow ou valores inválidos
        if (atual <= 0 || atual > 1e18) {
            *num_operacoes = -1;
            return atual;
        }
    }
    
    if (operacoes >= LIMITE_ITERACOES) {
        *num_operacoes = -1; // não convergiu
        return atual;
    }
    
    *num_operacoes = operacoes;
    return atual;
}

__global__ void testeConvergenciaKernel(long long inicio, long long fim, 
                                         long long *operacoes, int *convergiu) {
    long long idx = blockIdx.x * blockDim.x + threadIdx.x;
    long long numero = inicio + idx;
    
    if (numero > fim) return;
    
    long long local_idx = numero - inicio;
    long long num_ops = 0;
    long long resultado = testarConvergencia(numero, &num_ops);
    
    if (num_ops == -1) {
        // Não convergiu ou erro
        convergiu[local_idx] = 0;
        operacoes[local_idx] = -1;
    } else if (resultado == 1) {
        // Convergiu para 1
        convergiu[local_idx] = 1;
        operacoes[local_idx] = num_ops;
    } else {
        // Resultado inesperado
        convergiu[local_idx] = 0;
        operacoes[local_idx] = -1;
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
    
    clock_t inicio_tempo = clock();
    printf("Threads por bloco: %d\n", threadsPorBloco);
    printf("Testando conjectura de divisao/soma de digitos\n");
    printf("Intervalo: 1 ate %lld\n\n", fim);
    
    long long total_testados = 0;
    long long total_convergiu = 0;
    long long soma_operacoes = 0;
    long long max_operacoes = 0;
    long long num_max_operacoes = 0;
    
    // Estatísticas de distribuição
    long long hist_ops[21] = {0}; // 0-10, 11-20, ..., 91-100, 100+
    
    for (long long lote_inicio = 1; lote_inicio <= fim; lote_inicio += max_numeros_por_lote) {
        long long lote_fim = (lote_inicio + max_numeros_por_lote - 1 < fim) ? 
                              lote_inicio + max_numeros_por_lote - 1 : fim;
        long long numeros_neste_lote = lote_fim - lote_inicio + 1;
        
        int blocos = (int)((numeros_neste_lote + threadsPorBloco - 1) / threadsPorBloco);
        
        long long *d_operacoes = NULL;
        int *d_convergiu = NULL;
        CUDA_CHECK(cudaMalloc(&d_operacoes, numeros_neste_lote * sizeof(long long)));
        CUDA_CHECK(cudaMalloc(&d_convergiu, numeros_neste_lote * sizeof(int)));
        CUDA_CHECK(cudaMemset(d_operacoes, 0, numeros_neste_lote * sizeof(long long)));
        CUDA_CHECK(cudaMemset(d_convergiu, 0, numeros_neste_lote * sizeof(int)));
        
        testeConvergenciaKernel<<<blocos, threadsPorBloco>>>(lote_inicio, lote_fim, 
                                                               d_operacoes, d_convergiu);
        
        CUDA_CHECK(cudaDeviceSynchronize());
        CUDA_CHECK(cudaGetLastError());
        
        long long *h_operacoes = NULL;
        int *h_convergiu = NULL;
        CUDA_CHECK(cudaMallocHost((void**)&h_operacoes, numeros_neste_lote * sizeof(long long)));
        CUDA_CHECK(cudaMallocHost((void**)&h_convergiu, numeros_neste_lote * sizeof(int)));
        CUDA_CHECK(cudaMemcpy(h_operacoes, d_operacoes, numeros_neste_lote * sizeof(long long), 
                             cudaMemcpyDeviceToHost));
        CUDA_CHECK(cudaMemcpy(h_convergiu, d_convergiu, numeros_neste_lote * sizeof(int), 
                             cudaMemcpyDeviceToHost));
        
        // Processa resultados
        bool encontrouFalha = false;
        for (long long j = 0; j < numeros_neste_lote; ++j) {
            long long numero = lote_inicio + j;
            
            if (h_convergiu[j] == 0) {
                printf("FALHA: O numero %lld NAO convergiu para 1!\n", numero);
                printf("       Numero de operacoes: %lld\n", h_operacoes[j]);
                encontrouFalha = true;
                break;
            }
            
            total_convergiu++;
            long long ops = h_operacoes[j];
            soma_operacoes += ops;
            
            if (ops > max_operacoes) {
                max_operacoes = ops;
                num_max_operacoes = numero;
            }
            
            // Histograma
            int idx_hist = (ops <= 100) ? (ops / 5) : 20;
            hist_ops[idx_hist]++;
        }
        
        total_testados += numeros_neste_lote;
        
        CUDA_CHECK(cudaFreeHost(h_operacoes));
        CUDA_CHECK(cudaFreeHost(h_convergiu));
        CUDA_CHECK(cudaFree(d_operacoes));
        CUDA_CHECK(cudaFree(d_convergiu));
        
        // Exibe progresso
        double media = (total_convergiu > 0) ? (double)soma_operacoes / total_convergiu : 0;
        printf("Progresso: ate %lld | Testados: %lld | Convergencia: 100%% | Media ops: %.2f | Max ops: %lld (n=%lld)\n",
               lote_fim, total_testados, media, max_operacoes, num_max_operacoes);
        
        if (encontrouFalha) {
            printf("\nInterrompendo execucao devido a falha detectada.\n");
            
            // Mostra estatísticas parciais
            printf("\n========== ESTATISTICAS PARCIAIS ==========\n");
            printf("Total testados: %lld\n", total_testados);
            printf("Total que convergiram: %lld (%.2f%%)\n", 
                   total_convergiu, 100.0 * total_convergiu / total_testados);
            printf("=============================================\n");
            return;
        }
    }
    
    clock_t fim_tempo = clock();
    double tempo_decorrido = (double)(fim_tempo - inicio_tempo) / CLOCKS_PER_SEC;
    double media_operacoes = (total_convergiu > 0) ? (double)soma_operacoes / total_convergiu : 0;
    
    printf("\n========== RESULTADO FINAL ==========\n");
    printf("Total de numeros testados: %lld\n", total_testados);
    printf("Total que convergiram para 1: %lld (100%%)\n", total_convergiu);
    printf("\nESTATISTICAS DE OPERACOES:\n");
    printf("Media de operacoes: %.2f\n", media_operacoes);
    printf("Maximo de operacoes: %lld (para n = %lld)\n", max_operacoes, num_max_operacoes);
    
    printf("\nDISTRIBUICAO DE OPERACOES:\n");
    for (int i = 0; i < 20; i++) {
        if (hist_ops[i] > 0) {
            printf("  %3d-%3d operacoes: %10lld numeros (%.2f%%)\n", 
                   i*5, i*5+4, hist_ops[i], 100.0 * hist_ops[i] / total_testados);
        }
    }
    if (hist_ops[20] > 0) {
        printf("  100+    operacoes: %10lld numeros (%.2f%%)\n", 
               hist_ops[20], 100.0 * hist_ops[20] / total_testados);
    }
    
    printf("\nTempo total: %.2f segundos\n", tempo_decorrido);
    printf("Throughput: %.0f numeros/segundo\n", total_testados / tempo_decorrido);
    printf("====================================\n");
    
    if (total_convergiu == total_testados) {
        printf("\n*** CONJECTURA VERIFICADA: Todos os %lld numeros convergiram para 1! ***\n", 
               total_testados);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <fim_do_intervalo>\n", argv[0]);
        printf("Exemplo: %s 100000\n", argv[0]);
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
        printf("Nenhuma GPU CUDA encontrada.\n");
        return 1;
    }
    
    cudaDeviceProp prop;
    CUDA_CHECK(cudaGetDeviceProperties(&prop, 0));
    printf("GPU: %s\n", prop.name);
    printf("Compute Capability: %d.%d\n\n", prop.major, prop.minor);
    
    testeIntervalo(fim);
    return 0;
}

// Para compilar: nvcc -O3 TesteConjecturaDivisaoSoma.cu -o TesteConjecturaDivisaoSoma
// Para rodar: ./TesteConjecturaDivisaoSoma 100000
