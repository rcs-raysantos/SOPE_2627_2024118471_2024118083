# Compilador e flags
CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lreadline

# Nome do executável final
TARGET = parse

# Ficheiros de código fonte e objetos
SRCS = main.c parse.c
OBJS = $(SRCS:.c=.o)
HEADERS = header.h

# Regra principal (padrão)
all: $(TARGET)

# Compilação do executável final
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# Compilação dos ficheiros objeto (.o) a partir dos (.c)
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza dos ficheiros gerados
clean:
	rm -f $(OBJS) $(TARGET)

# Declaração de regras que não correspondem a ficheiros
.PHONY: all clean
