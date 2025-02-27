### FLAGS DE COMPILACION ####

CC=	gcc

### FLAGS PARA DEBUGGING ####
CFLAGS= 	-Wall -Wextra -Wdouble-promotion -Wno-unused-function -Wno-sign-conversion -Werror -fsanitize=undefined -std=gnu23 -O0 -ggdb

### COMPILACION ###

PROG= 	prueba
SRC= 	prueba.c
LIBS=	-pthread 

$(PROG): 	$(SRC)
		$(CC) $(CFLAGS) $(SRC) $(LIBS) -o $@
clean: 
	rm -f $(PROG) *.dSYM *~
all: 	$(PROG)
