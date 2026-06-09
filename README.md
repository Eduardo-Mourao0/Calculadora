# Avaliador de expressoes numericas

Trabalho pratico em C para avaliar expressoes em notacao pos-fixa e converter para as formas infixa e prefixa usando pilhas.

## Funcionalidades iniciais

- Operadores binarios: `+`, `-`, `*`, `/`, `%`, `^`.
- Funcoes unarias: `raiz`, `sen`, `cos`, `tg`, `log`.
- `sen`, `cos` e `tg` usam angulos em graus.
- Tokens devem ser separados por espaco.
- Tratamento de erros para token invalido, operandos insuficientes/sobrando, divisao por zero e dominio matematico invalido.

## Compilacao

Com `make`:

```sh
make
```

Ou diretamente com `gcc`:

```sh
gcc -std=c11 -Wall -Wextra -pedantic main.c calculadora.c -lm -o calculadora
```

## Uso

```sh
./calculadora
```

No Windows, caso compilado como `.exe`:

```sh
calculadora.exe
```

Digite `testes` no programa para executar os exemplos do enunciado.

## Testes adicionais

Tambem existe um arquivo de testes automatizados com casos que nao aparecem diretamente no enunciado:

- modulo;
- raiz quadrada;
- subtracao com resultado negativo;
- numero negativo;
- expressao aninhada;
- divisao por zero;
- raiz de numero negativo;
- logaritmo invalido;
- tangente indefinida;
- operandos insuficientes;
- operandos sobrando;
- token invalido.

Para compilar e executar:

```sh
make test
```

## Testes do enunciado

| Pos-fixa | Valor esperado |
| --- | --- |
| `3 4 + 5 *` | `35` |
| `7 2 * 4 +` | `18` |
| `8 5 2 4 + * +` | `38` |
| `6 2 / 3 + 4 *` | `24` |
| `9 5 2 8 * 4 + * +` | `109` |
| `2 3 + log 5 /` | aprox. `0.14` |
| `10 log 3 ^ 2 +` | `3` |
| `45 60 + 30 cos *` | aprox. `90.93` |
| `0.5 45 sen 2 ^ +` | `1` |
