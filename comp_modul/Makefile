CFLAGS+= -Wall -Werror -std=gnu99 -g
LDFLAGS=-lpthread

# HW=prgsem
BINARIES=comp_modul

CFLAGS+=$(shell sdl2-config --cflags)
LDFLAGS+=$(shell sdl2-config --libs) -lSDL2_image -lm

   
all: $(BINARIES)

OBJS=$(patsubst %.c,%.o,$(wildcard *.c))

comp_modul: $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(BINARIES) $(OBJS)