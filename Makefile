EXE=shadertemp
CC=cc
LIBS=-lm -lglfw -lGL
FLAGS=-Wall -Wextra -Werror -I./glad/include/
CFILES=./glad/src/glad.c main.c

.PHONY: all clean

all: $(EXE)

$(EXE): main.c
	$(CC) $(FLAGS) -o $(EXE) $(LIBS) $(CFILES)

clean:
	rm $(EXE)
