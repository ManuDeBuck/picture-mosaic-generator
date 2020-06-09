CC=	gcc
CFLAG= -O3
SRCS= main.c stb_image/stb_image.h
OBJ= main.o

collage.obj:	$(SRCS)
	$(CC) $(CFLAG) -c $(SRCS)

collage:	$(OBJ)
	$(CC)  $(CFLAG) -o Collage main.o stb_image/stb_image.h -lm

clean:
	rm -rf *.o Collage* *~
