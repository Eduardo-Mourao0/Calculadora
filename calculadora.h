#ifndef CALCULADORA_H
#define CALCULADORA_H

#include <stddef.h>

#define CALC_TEXTO_MAX 512
#define CALC_ERRO_MAX 160

typedef enum {
    CALC_OK = 0,
    CALC_ERRO_ENTRADA_VAZIA,
    CALC_ERRO_TOKEN_INVALIDO,
    CALC_ERRO_OPERANDOS_INSUFICIENTES,
    CALC_ERRO_OPERANDOS_SOBRANDO,
    CALC_ERRO_DIVISAO_ZERO,
    CALC_ERRO_DOMINIO_MATEMATICO,
    CALC_ERRO_EXPRESSAO_GRANDE
} CalcStatus;

typedef struct {
    CalcStatus status;
    double valor;
    char infixa[CALC_TEXTO_MAX];
    char prefixa[CALC_TEXTO_MAX];
    char erro[CALC_ERRO_MAX];
} CalcResultado;

CalcResultado calcular_posfixa(const char *expressao);
const char *calc_status_texto(CalcStatus status);

#endif
