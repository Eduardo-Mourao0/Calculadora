#include "calculadora.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PILHA_MAX 128
#define PI 3.14159265358979323846

typedef struct {
    double itens[PILHA_MAX];
    int topo;
} PilhaDouble;

typedef struct {
    char itens[PILHA_MAX][CALC_TEXTO_MAX];
    int topo;
} PilhaTexto;

static void definir_erro(CalcResultado *resultado, CalcStatus status, const char *detalhe)
{
    resultado->status = status;
    snprintf(resultado->erro, sizeof(resultado->erro), "%s%s%s", calc_status_texto(status), detalhe ? ": " : "", detalhe ? detalhe : "");
}

static int pilha_double_push(PilhaDouble *pilha, double valor)
{
    if (pilha->topo >= PILHA_MAX) {
        return 0;
    }
    pilha->itens[pilha->topo++] = valor;
    return 1;
}

static int pilha_double_pop(PilhaDouble *pilha, double *valor)
{
    if (pilha->topo <= 0) {
        return 0;
    }
    *valor = pilha->itens[--pilha->topo];
    return 1;
}

static int pilha_texto_push(PilhaTexto *pilha, const char *texto)
{
    if (pilha->topo >= PILHA_MAX || strlen(texto) >= CALC_TEXTO_MAX) {
        return 0;
    }
    strcpy(pilha->itens[pilha->topo++], texto);
    return 1;
}

static int pilha_texto_pop(PilhaTexto *pilha, char destino[CALC_TEXTO_MAX])
{
    if (pilha->topo <= 0) {
        return 0;
    }
    strcpy(destino, pilha->itens[--pilha->topo]);
    return 1;
}

static int eh_operador_binario(const char *token)
{
    return strlen(token) == 1 && strchr("+-*/%^", token[0]) != NULL;
}

static int eh_funcao_unaria(const char *token)
{
    return strcmp(token, "raiz") == 0 ||
           strcmp(token, "sen") == 0 ||
           strcmp(token, "cos") == 0 ||
           strcmp(token, "tg") == 0 ||
           strcmp(token, "log") == 0;
}

static int ler_numero(const char *token, double *valor)
{
    char *fim = NULL;
    errno = 0;
    *valor = strtod(token, &fim);
    return errno == 0 && fim != token && *fim == '\0';
}

static int montar_binario(char destino[CALC_TEXTO_MAX], const char *esq, const char *op, const char *dir)
{
    return snprintf(destino, CALC_TEXTO_MAX, "(%s %s %s)", esq, op, dir) < CALC_TEXTO_MAX;
}

static int montar_prefixo_binario(char destino[CALC_TEXTO_MAX], const char *op, const char *esq, const char *dir)
{
    return snprintf(destino, CALC_TEXTO_MAX, "%s %s %s", op, esq, dir) < CALC_TEXTO_MAX;
}

static int montar_unario(char destino[CALC_TEXTO_MAX], const char *funcao, const char *operando)
{
    return snprintf(destino, CALC_TEXTO_MAX, "%s(%s)", funcao, operando) < CALC_TEXTO_MAX;
}

static int montar_prefixo_unario(char destino[CALC_TEXTO_MAX], const char *funcao, const char *operando)
{
    return snprintf(destino, CALC_TEXTO_MAX, "%s %s", funcao, operando) < CALC_TEXTO_MAX;
}

static int aplicar_binario(const char *op, double a, double b, double *saida, CalcResultado *resultado)
{
    switch (op[0]) {
    case '+':
        *saida = a + b;
        return 1;
    case '-':
        *saida = a - b;
        return 1;
    case '*':
        *saida = a * b;
        return 1;
    case '/':
        if (b == 0.0) {
            definir_erro(resultado, CALC_ERRO_DIVISAO_ZERO, "divisao por zero");
            return 0;
        }
        *saida = a / b;
        return 1;
    case '%':
        if (b == 0.0) {
            definir_erro(resultado, CALC_ERRO_DIVISAO_ZERO, "modulo por zero");
            return 0;
        }
        *saida = fmod(a, b);
        return 1;
    case '^':
        errno = 0;
        *saida = pow(a, b);
        if (errno != 0 || !isfinite(*saida)) {
            definir_erro(resultado, CALC_ERRO_DOMINIO_MATEMATICO, "potencia fora do dominio");
            return 0;
        }
        return 1;
    default:
        definir_erro(resultado, CALC_ERRO_TOKEN_INVALIDO, op);
        return 0;
    }
}

static int aplicar_unario(const char *funcao, double a, double *saida, CalcResultado *resultado)
{
    if (strcmp(funcao, "raiz") == 0) {
        if (a < 0.0) {
            definir_erro(resultado, CALC_ERRO_DOMINIO_MATEMATICO, "raiz de numero negativo");
            return 0;
        }
        *saida = sqrt(a);
    } else if (strcmp(funcao, "sen") == 0) {
        *saida = sin(a * PI / 180.0);
    } else if (strcmp(funcao, "cos") == 0) {
        *saida = cos(a * PI / 180.0);
    } else if (strcmp(funcao, "tg") == 0) {
        double c = cos(a * PI / 180.0);
        if (fabs(c) < 1e-12) {
            definir_erro(resultado, CALC_ERRO_DOMINIO_MATEMATICO, "tangente indefinida");
            return 0;
        }
        *saida = tan(a * PI / 180.0);
    } else if (strcmp(funcao, "log") == 0) {
        if (a <= 0.0) {
            definir_erro(resultado, CALC_ERRO_DOMINIO_MATEMATICO, "logaritmo de numero nao positivo");
            return 0;
        }
        *saida = log10(a);
    } else {
        definir_erro(resultado, CALC_ERRO_TOKEN_INVALIDO, funcao);
        return 0;
    }

    return 1;
}

CalcResultado calcular_posfixa(const char *expressao)
{
    CalcResultado resultado = {0};
    PilhaDouble valores = {{0}, 0};
    PilhaTexto infixas = {{{0}}, 0};
    PilhaTexto prefixas = {{{0}}, 0};
    char copia[CALC_TEXTO_MAX];
    char *token = NULL;

    resultado.status = CALC_OK;

    if (expressao == NULL || expressao[0] == '\0') {
        definir_erro(&resultado, CALC_ERRO_ENTRADA_VAZIA, NULL);
        return resultado;
    }

    if (strlen(expressao) >= sizeof(copia)) {
        definir_erro(&resultado, CALC_ERRO_EXPRESSAO_GRANDE, NULL);
        return resultado;
    }

    strcpy(copia, expressao);
    token = strtok(copia, " \t\r\n");

    while (token != NULL) {
        double numero = 0.0;

        if (ler_numero(token, &numero)) {
            if (!pilha_double_push(&valores, numero) ||
                !pilha_texto_push(&infixas, token) ||
                !pilha_texto_push(&prefixas, token)) {
                definir_erro(&resultado, CALC_ERRO_EXPRESSAO_GRANDE, NULL);
                return resultado;
            }
        } else if (eh_operador_binario(token)) {
            double b = 0.0;
            double a = 0.0;
            double valor = 0.0;
            char dir[CALC_TEXTO_MAX];
            char esq[CALC_TEXTO_MAX];
            char pref_dir[CALC_TEXTO_MAX];
            char pref_esq[CALC_TEXTO_MAX];
            char infixa[CALC_TEXTO_MAX];
            char prefixa[CALC_TEXTO_MAX];

            if (!pilha_double_pop(&valores, &b) || !pilha_double_pop(&valores, &a) ||
                !pilha_texto_pop(&infixas, dir) || !pilha_texto_pop(&infixas, esq) ||
                !pilha_texto_pop(&prefixas, pref_dir) || !pilha_texto_pop(&prefixas, pref_esq)) {
                definir_erro(&resultado, CALC_ERRO_OPERANDOS_INSUFICIENTES, token);
                return resultado;
            }

            if (!aplicar_binario(token, a, b, &valor, &resultado)) {
                return resultado;
            }
            if (!montar_binario(infixa, esq, token, dir) ||
                !montar_prefixo_binario(prefixa, token, pref_esq, pref_dir) ||
                !pilha_double_push(&valores, valor) ||
                !pilha_texto_push(&infixas, infixa) ||
                !pilha_texto_push(&prefixas, prefixa)) {
                definir_erro(&resultado, CALC_ERRO_EXPRESSAO_GRANDE, NULL);
                return resultado;
            }
        } else if (eh_funcao_unaria(token)) {
            double a = 0.0;
            double valor = 0.0;
            char operando[CALC_TEXTO_MAX];
            char pref_operando[CALC_TEXTO_MAX];
            char infixa[CALC_TEXTO_MAX];
            char prefixa[CALC_TEXTO_MAX];

            if (!pilha_double_pop(&valores, &a) ||
                !pilha_texto_pop(&infixas, operando) ||
                !pilha_texto_pop(&prefixas, pref_operando)) {
                definir_erro(&resultado, CALC_ERRO_OPERANDOS_INSUFICIENTES, token);
                return resultado;
            }
            if (!aplicar_unario(token, a, &valor, &resultado)) {
                return resultado;
            }
            if (!montar_unario(infixa, token, operando) ||
                !montar_prefixo_unario(prefixa, token, pref_operando) ||
                !pilha_double_push(&valores, valor) ||
                !pilha_texto_push(&infixas, infixa) ||
                !pilha_texto_push(&prefixas, prefixa)) {
                definir_erro(&resultado, CALC_ERRO_EXPRESSAO_GRANDE, NULL);
                return resultado;
            }
        } else {
            definir_erro(&resultado, CALC_ERRO_TOKEN_INVALIDO, token);
            return resultado;
        }

        token = strtok(NULL, " \t\r\n");
    }

    if (valores.topo != 1 || infixas.topo != 1 || prefixas.topo != 1) {
        definir_erro(&resultado, CALC_ERRO_OPERANDOS_SOBRANDO, NULL);
        return resultado;
    }

    resultado.valor = valores.itens[0];
    strcpy(resultado.infixa, infixas.itens[0]);
    strcpy(resultado.prefixa, prefixas.itens[0]);
    return resultado;
}

const char *calc_status_texto(CalcStatus status)
{
    switch (status) {
    case CALC_OK:
        return "ok";
    case CALC_ERRO_ENTRADA_VAZIA:
        return "entrada vazia";
    case CALC_ERRO_TOKEN_INVALIDO:
        return "token invalido";
    case CALC_ERRO_OPERANDOS_INSUFICIENTES:
        return "operandos insuficientes";
    case CALC_ERRO_OPERANDOS_SOBRANDO:
        return "operandos sobrando";
    case CALC_ERRO_DIVISAO_ZERO:
        return "divisao por zero";
    case CALC_ERRO_DOMINIO_MATEMATICO:
        return "erro de dominio matematico";
    case CALC_ERRO_EXPRESSAO_GRANDE:
        return "expressao grande demais";
    default:
        return "erro desconhecido";
    }
}
