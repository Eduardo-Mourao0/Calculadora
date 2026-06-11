#include "calculadora.h"

#include <stdio.h>
#include <string.h>

#define ENTRADA_MAX 512

static void remover_espacos_das_bordas(char *texto)
{
    size_t inicio = 0;
    size_t fim = strlen(texto);

    while (texto[inicio] == ' ' || texto[inicio] == '\t') {
        inicio++;
    }
    while (fim > inicio && (texto[fim - 1] == ' ' || texto[fim - 1] == '\t')) {
        fim--;
    }

    if (inicio > 0) {
        memmove(texto, texto + inicio, fim - inicio);
    }
    texto[fim - inicio] = '\0';
}

static void executar_teste(const char *expressao)
{
    char copia_infixa[ENTRADA_MAX];
    char copia_valor[ENTRADA_MAX];
    char *infixa;
    float valor;

    printf("Posfixa: %s\n", expressao);

    strncpy(copia_infixa, expressao, ENTRADA_MAX - 1);
    copia_infixa[ENTRADA_MAX - 1] = '\0';
    strncpy(copia_valor, expressao, ENTRADA_MAX - 1);
    copia_valor[ENTRADA_MAX - 1] = '\0';

    infixa = getInFixa(copia_infixa);
    if (infixa == NULL) {
        printf("Expressao invalida.\n\n");
        return;
    }

    valor = getValor(copia_valor);

    printf("Infixa : %s\n", infixa);
    printf("Valor  : %.10g\n\n", valor);
}

static void executar_testes_do_enunciado(void)
{
    const char *testes[] = {
        "3 4 + 5 *",
        "7 2 * 4 +",
        "8 5 2 4 + * +",
        "6 2 / 3 + 4 *",
        "9 5 2 8 * 4 + * +",
        "2 3 + log 5 /",
        "10 log 3 ^ 2 +",
        "45 60 + 30 cos *",
        "0.5 45 sen 2 ^ +"
    };
    size_t total = sizeof(testes) / sizeof(testes[0]);
    size_t i;

    for (i = 0; i < total; i++) {
        executar_teste(testes[i]);
    }
}

int main(void)
{
    char entrada[ENTRADA_MAX];

    printf("Avaliador de expressoes numericas pos-fixas\n");
    printf("Operadores: + - * / %% ^ | Funcoes: raiz sen cos tg log\n");
    printf("Digite uma expressao com tokens separados por espaco.\n");
    printf("Digite 'testes' para executar os exemplos ou 'sair' para encerrar.\n\n");

    while (1) {
        printf("> ");
        if (fgets(entrada, sizeof(entrada), stdin) == NULL) {
            break;
        }

        entrada[strcspn(entrada, "\r\n")] = '\0';
        remover_espacos_das_bordas(entrada);

        if (strcmp(entrada, "sair") == 0) {
            break;
        }
        if (strcmp(entrada, "testes") == 0) {
            executar_testes_do_enunciado();
            continue;
        }

        executar_teste(entrada);
    }

    return 0;
}
