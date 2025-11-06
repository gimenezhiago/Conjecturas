#include <stdio.h>
#include <stdlib.h>

unsigned long long substituirZeros(unsigned long long n) {
    if (n == 0) {
        return 2;
    }

    unsigned long long resultado = 0;
    unsigned long long multiplicador = 1;

    while (n > 0) {
        unsigned long long digito = n % 10;

        if (digito == 0) {
            digito = 2;
        }

        resultado = digito * multiplicador + resultado;
        multiplicador *= 10;
        n /= 10;
    }
    return resultado;
}

unsigned long long produtoDigitos(unsigned long long n) {
    unsigned long long produto = 1;
    for (; n > 0; n /= 10) {
        unsigned long long digito = n % 10;
        produto = produto * digito;
    }
    return produto;
}

unsigned long long aplicarRegras(unsigned long long n) {
    unsigned long long numeroModificado = substituirZeros(n);

    unsigned long long produto = produtoDigitos(numeroModificado);

    if (produto == 1) {
        return numeroModificado + 1;
    }

    unsigned long long soma = numeroModificado + produto;

    if (numeroModificado % produto == 0) {
        return numeroModificado/produto;
    } else {
        return soma;
    }
}

unsigned long long testarNumero(unsigned long long n) {
    const unsigned long long LIMITE = 10000;
    unsigned long long atual = n;
    unsigned long long contador = 0;

    while (contador <= LIMITE) {
        if (atual == 1) {
            return contador;
        }
        
        if (atual == 774) {
            return 0;
        }
        
        if (atual == 2238) {
            return 0;
        }

        atual = aplicarRegras(atual);
        contador++;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <numero>\n", argv[0]);
        return 1;
    }

    unsigned long long limite = strtoull(argv[1], NULL, 10);
    if (limite <= 0) {
        printf("Parametro invalido: %s\n", argv[1]);
        return 1;
    }

    unsigned long long maxPassos = 0;
    unsigned long long numeroMaxPassos = 0;

    printf("Testando numeros de 1 ate %llu:\n\n", limite);

    for (unsigned long long i = 1; i <= limite; i++) {
        unsigned long long resultado = testarNumero(i);
        if (resultado > maxPassos) {
            maxPassos = resultado;
            numeroMaxPassos = i;
        }
    }

    printf("Numero com maior quantidade de passos: %llu\n", numeroMaxPassos);
    printf("Quantidade de passos: %llu\n", maxPassos);

    return 0;
}
