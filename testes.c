#include "calculadora.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    const char *nome;
    const char *posfixa;
    const char *infixa;
    float valor;
} CasoValido;

typedef struct {
    const char *nome;
    const char *posfixa;
} CasoInvalido;

static int quase_igual(float a, float b)
{
    return fabsf(a - b) <= 0.001f;
}

static int testar_valido(CasoValido caso)
{
    char entrada_infixa[512];
    char entrada_valor[512];
    char *infixa;
    float valor;
    int ok_infixa;
    int ok_valor;

    strcpy(entrada_infixa, caso.posfixa);
    strcpy(entrada_valor, caso.posfixa);

    infixa = getInFixa(entrada_infixa);
    valor = getValor(entrada_valor);

    ok_infixa = infixa != NULL && strcmp(infixa, caso.infixa) == 0;
    ok_valor = quase_igual(valor, caso.valor);

    if (ok_infixa && ok_valor) {
        printf("[OK] %s\n", caso.nome);
        return 1;
    }

    printf("[FALHOU] %s\n", caso.nome);
    printf("  Posfixa: %s\n", caso.posfixa);
    printf("  Infixa esperada: %s\n", caso.infixa);
    printf("  Infixa obtida  : %s\n", infixa ? infixa : "NULL");
    printf("  Valor esperado : %.6f\n", caso.valor);
    printf("  Valor obtido   : %.6f\n", valor);
    return 0;
}

static int testar_invalido(CasoInvalido caso)
{
    char entrada_infixa[512];
    char entrada_valor[512];
    char *infixa;
    float valor;

    strcpy(entrada_infixa, caso.posfixa);
    strcpy(entrada_valor, caso.posfixa);

    infixa = getInFixa(entrada_infixa);
    valor = getValor(entrada_valor);

    if (infixa == NULL && valor == 0.0f) {
        printf("[OK] %s\n", caso.nome);
        return 1;
    }

    printf("[FALHOU] %s\n", caso.nome);
    printf("  Posfixa: %s\n", caso.posfixa);
    printf("  Infixa obtida: %s\n", infixa ? infixa : "NULL");
    printf("  Valor obtido : %.6f\n", valor);
    return 0;
}

int main(void)
{
    CasoValido validos[] = {
        {"teste 1", "3 4 + 5 *", "(3+4)*5", 35.0f},
        {"teste 2", "7 2 * 4 +", "7*2+4", 18.0f},
        {"teste 3", "8 5 2 4 + * +", "8+5*(2+4)", 38.0f},
        {"teste 4", "6 2 / 3 + 4 *", "(6/2+3)*4", 24.0f},
        {"teste 5", "9 5 2 8 * 4 + * +", "9+5*(2*8+4)", 109.0f},
        {"teste 6", "2 3 + log 5 /", "log(2+3)/5", 0.139794f},
        {"teste 7", "10 log 3 ^ 2 +", "log(10)^3+2", 3.0f},
        {"teste 8", "45 60 + 30 cos *", "(45+60)*cos(30)", 90.932667f},
        {"teste 9", "0.5 45 sen 2 ^ +", "0.5+sen(45)^2", 1.0f},
        {"modulo", "10 3 %", "10%3", 1.0f},
        {"raiz", "9 raiz 2 +", "raiz(9)+2", 5.0f}
    };
    CasoInvalido invalidos[] = {
        {"operandos insuficientes", "3 4 + +"},
        {"operandos sobrando", "3 4"},
        {"divisao por zero", "10 0 /"},
        {"modulo por zero", "10 0 %"},
        {"raiz invalida", "-9 raiz"},
        {"log invalido", "0 log"},
        {"token invalido", "3 abc +"}
    };
    size_t total_validos = sizeof(validos) / sizeof(validos[0]);
    size_t total_invalidos = sizeof(invalidos) / sizeof(invalidos[0]);
    size_t aprovados = 0;
    size_t total = total_validos + total_invalidos;
    size_t i;

    for (i = 0; i < total_validos; i++) {
        aprovados += testar_valido(validos[i]);
    }

    for (i = 0; i < total_invalidos; i++) {
        aprovados += testar_invalido(invalidos[i]);
    }

    printf("\n%zu/%zu testes aprovados.\n", aprovados, total);
    return aprovados == total ? 0 : 1;
}
