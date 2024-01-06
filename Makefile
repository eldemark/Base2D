#
# Makefile for Base2D
#

CC := clang
CFLAGS := -g -ggdb -O2 -Wall
PROGRAM := basic2d
INC := -I. -Iglad/include
LIB := -lX11 -lGLX
OBJ := main.o glad/src/glad.o

.SUFFIXES:
.SUFFIXES: .o .c .h

# Compile
.c.o:
	$(CC) -c $(CFLAGS) $(INC) -o $@ $<

all: $(PROGRAM)

# Link
$(PROGRAM): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LIB)

install: $(PROGRAM)
	echo "no install"

clean:
	rm -f $(OBJ)
	rm -f $(PROGRAM)
