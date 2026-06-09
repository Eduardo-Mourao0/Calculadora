CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -pedantic
LDLIBS = -lm
TARGET = calculadora
OBJS = main.o calculadora.o
TEST_TARGET = testes

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDLIBS)

main.o: main.c calculadora.h
	$(CC) $(CFLAGS) -c main.c

calculadora.o: calculadora.c calculadora.h
	$(CC) $(CFLAGS) -c calculadora.c

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): testes.c calculadora.c calculadora.h
	$(CC) $(CFLAGS) -o $(TEST_TARGET) testes.c calculadora.c $(LDLIBS)

clean:
	del /Q $(OBJS) $(TARGET).exe $(TEST_TARGET).exe 2>NUL || exit 0

.PHONY: all test clean
