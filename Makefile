CC = clang
CFLAGS = -g -Wall -O2 -MD -MP -I./compat
LDFLAGS =

SRCS = compat/strlcpy.c compat/strlcat.c
SRCS+= query.c maildir.c mail.c
OBJS = $(SRCS:.c=.o)

all: lsmail edmail

lsmail: lsmail.o $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

edmail: edmail.o $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf lsmail edmail $(OBJS) $(OBJS:.o=.d)

-include ${OBJS:.o=.d}

.PHONY: clean
