#include "calculadora.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    const char *nome;
    const char *expressao;
    double esperado;
    CalcStatus status_esperado;
} CasoTeste;

static int quase_igual(double a, double b)
{
    return fabs(a - b) < 1e-9;
}

static int testar_caso(CasoTeste caso)
{
    CalcResultado resultado = calcular_posfixa(caso.expressao);

    if (resultado.status != caso.status_esperado) {
        printf("[FALHOU] %s\n", caso.nome);
        printf("  Expressao: %s\n", caso.expressao);
        printf("  Status esperado: %s\n", calc_status_texto(caso.status_esperado));
        printf("  Status obtido  : %s\n", calc_status_texto(resultado.status));
        if (resultado.erro[0] != '\0') {
            printf("  Erro: %s\n", resultado.erro);
        }
        return 0;
    }

    if (resultado.status == CALC_OK && !quase_igual(resultado.valor, caso.esperado)) {
        printf("[FALHOU] %s\n", caso.nome);
        printf("  Expressao: %s\n", caso.expressao);
        printf("  Valor esperado: %.12g\n", caso.esperado);
        printf("  Valor obtido  : %.12g\n", resultado.valor);
        return 0;
    }

    printf("[OK] %s\n", caso.nome);
    return 1;
}

int main(void)
{
    CasoTeste casos[] = {
        {"modulo", "10 3 %", 1.0, CALC_OK},
        {"raiz quadrada", "9 raiz", 3.0, CALC_OK},
        {"subtracao negativa", "5 8 -", -3.0, CALC_OK},
        {"numero negativo", "-3 2 *", -6.0, CALC_OK},
        {"expressao aninhada", "2 3 4 + *", 14.0, CALC_OK},
        {"divisao por zero", "10 0 /", 0.0, CALC_ERRO_DIVISAO_ZERO},
        {"raiz invalida", "-9 raiz", 0.0, CALC_ERRO_DOMINIO_MATEMATICO},
        {"log invalido", "0 log", 0.0, CALC_ERRO_DOMINIO_MATEMATICO},
        {"tangente indefinida", "90 tg", 0.0, CALC_ERRO_DOMINIO_MATEMATICO},
        {"operandos insuficientes", "3 4 + +", 0.0, CALC_ERRO_OPERANDOS_INSUFICIENTES},
        {"operandos sobrando", "3 4", 0.0, CALC_ERRO_OPERANDOS_SOBRANDO},
        {"token invalido", "3 abc +", 0.0, CALC_ERRO_TOKEN_INVALIDO}
    };
    size_t total = sizeof(casos) / sizeof(casos[0]);
    size_t aprovados = 0;
    size_t i;

    for (i = 0; i < total; i++) {
        aprovados += testar_caso(casos[i]);
    }

    printf("\n%zu/%zu testes aprovados.\n", aprovados, total);
    return aprovados == total ? 0 : 1;
}
