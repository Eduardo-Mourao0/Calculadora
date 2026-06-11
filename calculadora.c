#include "calculadora.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PILHA_MAX 128
#define STR_MAX   512
#define PI        3.14159265358979323846

/* ---------- pilhas internas ---------- */

typedef struct {
    double itens[PILHA_MAX];
    int    topo;
} PilhaDouble;

typedef struct {
    char   texto[PILHA_MAX][STR_MAX];
    int    precedencia[PILHA_MAX];
    char   operador[PILHA_MAX];
    double valor[PILHA_MAX];
    int    topo;
} PilhaExpr;

static int pd_push(PilhaDouble *p, double v)
{
    if (p->topo >= PILHA_MAX) return 0;
    p->itens[p->topo++] = v;
    return 1;
}

static int pd_pop(PilhaDouble *p, double *v)
{
    if (p->topo <= 0) return 0;
    *v = p->itens[--p->topo];
    return 1;
}

static int pe_push(PilhaExpr *p, const char *s, int precedencia, char operador, double valor)
{
    if (p->topo >= PILHA_MAX || strlen(s) >= STR_MAX) return 0;
    strcpy(p->texto[p->topo], s);
    p->precedencia[p->topo] = precedencia;
    p->operador[p->topo] = operador;
    p->valor[p->topo] = valor;
    p->topo++;
    return 1;
}

static int pe_pop(PilhaExpr *p, char dst[STR_MAX], int *precedencia, char *operador, double *valor)
{
    if (p->topo <= 0) return 0;
    p->topo--;
    strcpy(dst, p->texto[p->topo]);
    *precedencia = p->precedencia[p->topo];
    *operador = p->operador[p->topo];
    *valor = p->valor[p->topo];
    return 1;
}

/* ---------- helpers de classificação ---------- */

static int eh_operador(const char *t)
{
    return strlen(t) == 1 && strchr("+-*/%^", t[0]) != NULL;
}

static int eh_funcao(const char *t)
{
    return strcmp(t, "raiz") == 0 ||
           strcmp(t, "sen")  == 0 ||
           strcmp(t, "cos")  == 0 ||
           strcmp(t, "tg")   == 0 ||
           strcmp(t, "log")  == 0;
}

static int ler_numero(const char *t, double *v)
{
    char *fim = NULL;
    errno = 0;
    *v = strtod(t, &fim);
    return errno == 0 && fim != t && *fim == '\0';
}

/* ---------- montagem de expressão infixa ---------- */

/* Retorna 1 se o operador precisa de parênteses ao ser aninhado */
static int precedencia_operador(char op)
{
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/' || op == '%') return 2;
    if (op == '^') return 3;
    return 4;
}

static int precisa_parenteses_esq(char op_pai, int prec_filho, char op_filho)
{
    int prec_pai = precedencia_operador(op_pai);

    if (prec_filho < prec_pai) return 1;
    if (op_pai == '^' && prec_filho == prec_pai) return 1;
    if (op_pai == '^' && op_filho == 'n') return 1;

    return 0;
}

static int precisa_parenteses_dir(char op_pai, int prec_filho, char op_filho)
{
    int prec_pai = precedencia_operador(op_pai);

    if (prec_filho < prec_pai) return 1;
    if (prec_filho > prec_pai) return 0;

    if (op_pai == '-' && (op_filho == '+' || op_filho == '-')) return 1;
    if ((op_pai == '/' || op_pai == '%') && prec_filho == prec_pai) return 1;
    if (op_pai == '*' && op_filho == '%') return 1;
    if (op_pai == '^' && op_filho == 'n') return 1;

    return 0;
}

static int copiar_operando(char dst[STR_MAX], const char *expr, int usar_parenteses)
{
    if (usar_parenteses) {
        return snprintf(dst, STR_MAX, "(%s)", expr) < STR_MAX;
    }

    return snprintf(dst, STR_MAX, "%s", expr) < STR_MAX;
}

static int montar_infixa(char dst[STR_MAX],
                         const char *esq,
                         int prec_esq,
                         char op_esq,
                         const char *op,
                         const char *dir,
                         int prec_dir,
                         char op_dir)
{
    char esq_final[STR_MAX];
    char dir_final[STR_MAX];

    if (!copiar_operando(esq_final, esq, precisa_parenteses_esq(op[0], prec_esq, op_esq))) return 0;
    if (!copiar_operando(dir_final, dir, precisa_parenteses_dir(op[0], prec_dir, op_dir))) return 0;

    return snprintf(dst, STR_MAX, "%s%s%s", esq_final, op, dir_final) < STR_MAX;
}

static int montar_infixa_unaria(char dst[STR_MAX], const char *fn, const char *operando)
{
    return snprintf(dst, STR_MAX, "%s(%s)", fn, operando) < STR_MAX;
}

/* ---------- avaliação ---------- */

static int aplicar_binario(char op, double a, double b, double *saida)
{
    switch (op) {
    case '+': *saida = a + b; return 1;
    case '-': *saida = a - b; return 1;
    case '*': *saida = a * b; return 1;
    case '/':
        if (b == 0.0) return 0;
        *saida = a / b; return 1;
    case '%':
        if (b == 0.0) return 0;
        *saida = fmod(a, b); return 1;
    case '^':
        errno = 0;
        *saida = pow(a, b);
        return errno == 0 && isfinite(*saida);
    default: return 0;
    }
}

static int aplicar_unario(const char *fn, double a, double *saida)
{
    if (strcmp(fn, "raiz") == 0) {
        if (a < 0.0) return 0;
        *saida = sqrt(a);
    } else if (strcmp(fn, "sen") == 0) {
        *saida = sin(a * PI / 180.0);
    } else if (strcmp(fn, "cos") == 0) {
        *saida = cos(a * PI / 180.0);
    } else if (strcmp(fn, "tg") == 0) {
        if (fabs(cos(a * PI / 180.0)) < 1e-12) return 0;
        *saida = tan(a * PI / 180.0);
    } else if (strcmp(fn, "log") == 0) {
        if (a <= 0.0) return 0;
        *saida = log10(a);
    } else {
        return 0;
    }
    return 1;
}

char *getInFixa(char *Str)
{
    static char resultado[STR_MAX];
    PilhaExpr   pilha = {{{0}}, {0}, {0}, {0}, 0};
    char        copia[STR_MAX];
    char        *token;

    if (Str == NULL || Str[0] == '\0') return NULL;
    if (strlen(Str) >= STR_MAX)        return NULL;

    strcpy(copia, Str);
    token = strtok(copia, " \t\r\n");

    while (token != NULL) {
        double num;

        if (ler_numero(token, &num)) {
            char op_numero = token[0] == '-' ? 'n' : '\0';
            if (!pe_push(&pilha, token, 4, op_numero, num)) return NULL;

        } else if (eh_operador(token)) {
            char dir[STR_MAX], esq[STR_MAX], infixa[STR_MAX];
            int prec_dir, prec_esq;
            char op_dir, op_esq;
            double valor_dir, valor_esq, valor;

            if (!pe_pop(&pilha, dir, &prec_dir, &op_dir, &valor_dir) ||
                !pe_pop(&pilha, esq, &prec_esq, &op_esq, &valor_esq)) return NULL;
            if (!aplicar_binario(token[0], valor_esq, valor_dir, &valor)) return NULL;
            if (!montar_infixa(infixa, esq, prec_esq, op_esq, token, dir, prec_dir, op_dir)) return NULL;
            if (!pe_push(&pilha, infixa, precedencia_operador(token[0]), token[0], valor)) return NULL;

        } else if (eh_funcao(token)) {
            char operando[STR_MAX], infixa[STR_MAX];
            int prec_operando;
            char op_operando;
            double valor_operando, valor;

            if (!pe_pop(&pilha, operando, &prec_operando, &op_operando, &valor_operando)) return NULL;
            if (!aplicar_unario(token, valor_operando, &valor)) return NULL;
            if (!montar_infixa_unaria(infixa, token, operando)) return NULL;
            if (!pe_push(&pilha, infixa, 4, 'F', valor)) return NULL;

        } else {
            return NULL; /* token desconhecido */
        }

        token = strtok(NULL, " \t\r\n");
    }

    if (pilha.topo != 1) return NULL;

    strncpy(resultado, pilha.texto[0], STR_MAX - 1);
    resultado[STR_MAX - 1] = '\0';
    return resultado;
}

float getValor(char *Str)
{
    PilhaDouble pilha = {{0}, 0};
    char        copia[STR_MAX];
    char        *token;

    if (Str == NULL || Str[0] == '\0') return 0.0f;
    if (strlen(Str) >= STR_MAX)        return 0.0f;

    strcpy(copia, Str);
    token = strtok(copia, " \t\r\n");

    while (token != NULL) {
        double num;

        if (ler_numero(token, &num)) {
            if (!pd_push(&pilha, num)) return 0.0f;

        } else if (eh_operador(token)) {
            double a, b, valor;
            if (!pd_pop(&pilha, &b) || !pd_pop(&pilha, &a)) return 0.0f;
            if (!aplicar_binario(token[0], a, b, &valor))   return 0.0f;
            if (!pd_push(&pilha, valor))                     return 0.0f;

        } else if (eh_funcao(token)) {
            double a, valor;
            if (!pd_pop(&pilha, &a))               return 0.0f;
            if (!aplicar_unario(token, a, &valor)) return 0.0f;
            if (!pd_push(&pilha, valor))           return 0.0f;

        } else {
            return 0.0f; /* token desconhecido */
        }

        token = strtok(NULL, " \t\r\n");
    }

    if (pilha.topo != 1) return 0.0f;

    return (float)pilha.itens[0];
}
