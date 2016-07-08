#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <err.h>
#include <termios.h>

#define MAX_BCC 10
#define MAX_CC 10

static char *argv0;

struct addrs {
	char *addr;
	struct addrs *next;
};

static struct addrs *bcc;
static struct addrs *cc;
static char *subject;
static char *from;
static int allow_empty = 1;

struct addrs *
addr_add(struct addrs *h, char *b)
{
	struct addrs *n, *a;

	n = calloc(1, sizeof(*n));
	if (!n)
		err(1, "calloc");
	n->addr = b;
	n->next = NULL;

	for (a = h; a && a->next; a = a->next);
	if (!a)
		h = n;
	else
		a->next = n;

	return h;
}

int
ask_subject() {
	size_t l;
	char *b;
	printf("Subject: ");
	getline(&b, &l, stdin);
	subject = b;
	return 0;
}

int
main(int argc, char *argv[])
{
	int c;
	struct addrs *a;

	argv0 = argv[0];

	while ((c = getopt(argc, argv, "b:c:E")) != -1) {
		switch (c) {
		case 'b': bcc = addr_add(bcc, optarg); break;
		case 'c': cc = addr_add(cc, optarg); break;
		case 'E': allow_empty--; break;
		case 'r': from = optarg; break;
		case 's': subject = optarg; break;
		}
	}

	for (a = cc; a; a = a->next)
		printf("CC: %s\n", a->addr);

	for (a = bcc; a; a = a->next)
		printf("BCC: %s\n", a->addr);

	if (!subject)
		ask_subject();

	return 0;
}
