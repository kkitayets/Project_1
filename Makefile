CC = gcc
CFLAGS = -g -Wall `pkg-config --cflags gtk+-3.0`
LDFLAGS = `pkg-config --libs gtk+-3.0`

all: gtk_app

gtk_app: main.c
	$(CC) -o gtk_app main.c $(CFLAGS) $(LDFLAGS)

clean:
	rm -f gtk_app
