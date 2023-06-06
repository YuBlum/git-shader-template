EXE=shadertemp
CC=cc
LIBS=-lm -lglfw
FLAGS=-Wall -Wextra -Werror -I./include/

.PHONY: all clean

all: $(EXE)

$(EXE): main.c
	$(CC) $(FLAGS) -o $(EXE) $(LIBS) main.c 

clean:
	rm $(EXE)
