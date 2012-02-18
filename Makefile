INCLUDE = -I"c:/Program Files (x86)/AMD APP/include"

COMPILERFLAGS = -std=c99 -Wall -pedantic 
CC = gcc
CFLAGS = $(COMPILERFLAGS) $(INCLUDE)
LIBRARIES = -L"c:/Program Files (x86)/AMD APP/lib/x86" -lOpenCL
OBJ = ocl.o
OUT = ocl.exe

$(OUT): $(OBJ)
	$(CC) $(OBJ) $(LIBRARIES) -o $(OUT)


clean:
	rm -f $(OBJ) $(OUT)

