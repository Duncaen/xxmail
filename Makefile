CC = clang
CFLAGS = -g -Wall -O2 -MD -MP -I./compat
LDFLAGS =

SRCS = compat/strlcpy.c compat/strlcat.c
SRCS+= query.c maildir.c mail.c lsmail.c
OBJS = $(SRCS:.c=.o)

lsmail: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -rf lsmail $(OBJS) $(OBJS:.o=.d)

-include ${OBJS:.o=.d}

.PHONY: clean
