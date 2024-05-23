# Nom de l'exécutable
TARGET = Game.out

# Compilateur et options de compilation
CC = gcc
CFLAGS = -Wall -Wextra -std=c11

# Fichiers sources
SRCS = Main.c Game.c Dictionary.c Loader.c Utils.c

# Fichiers objets
OBJS = $(SRCS:.c=.o)

# Règle par défaut : créer l'exécutable
all: $(TARGET)

# Règle pour créer l'exécutable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

# Règle pour compiler les fichiers .c en fichiers .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Règle pour nettoyer les fichiers générés
clean:
	rm -f $(OBJS) $(TARGET)

# Règle pour forcer la recompilation
.PHONY: clean all