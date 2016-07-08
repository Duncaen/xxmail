#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <err.h>

#include <sys/stat.h>

#include "mail.h"

static char *argv0;
static char *format;

static char default_format[] = "%i [%F] [%S]\n";

void
analyze_format(struct query *q)
{
	char *s;
	for (s = format; *s; s++) {
		if (*s == '\\')
			continue;
		switch (*++s) {
		case 's': /* state */ break;
		case 'i': /* index */ break;
		case 'S': q->hdr |= HDR_SUBJECT; break;
		case 'F': q->hdr |= HDR_FROM; break;
		}
	}
}

void
print_format(struct mail *m)
{
	char *s;
	for (s = format; *s; s++) {
		if (*s == '\\') {
			switch (*++s) {
			case 'a': putchar('\a'); break;
			case 'b': putchar('\b'); break;
			case 'f': putchar('\f'); break;
			case 'n': putchar('\n'); break;
			case 'r': putchar('\r'); break;
			case 't': putchar('\t'); break;
			case 'v': putchar('\v'); break;
			case '0': putchar('\0'); break;
			// TODO: \NNN
			default: putchar(*s);
			}
			continue;
		}
		if (*s != '%') {
			putchar(*s);
			continue;
		}
		switch (*++s) {
		case '%': putchar('%'); break;
		case 's': printf("%c", 'N'); break;
		case 'i': printf("%d", 0); break;
		case 'S': printf("%s", m->subject); break;
		case 'F':
			switch (*++s) {
			case '@': printf("%s", m->from); break;
			case 'N': printf("%s", m->from); break;
			default: s--; printf("%s", m->from);
			}
			break;
		default:
			putchar('%');
			putchar(*s);
		}
	}
}

static void
usage()
{
	fprintf(stderr, "Usage: %s [-f FMT] PATH...\n", argv0);
	exit(2);
}

int
main(int argc, char *argv[])
{
	int c, i;
	struct stat st;
	struct query *q;
	struct mailbox *mb;
	struct mail *m;

	argv0 = argv[0];
	format = default_format;

	while ((c = getopt(argc, argv, "f:")) != -1) {
		switch (c) {
		case 'f':
			format = optarg;
			break;
		default: usage();
		}
	}

	if (optind == argc)
		usage();

	q = query_init();

	for (i = optind; i < argc; i++) {
		if (stat(argv[i], &st) == -1)
			err(1, "stat");
		if (st.st_mode & S_IFDIR) {
			mb = maildir_init(argv[i]);
			query_add_mailbox(q, mb);
		}
	}

	analyze_format(q);
	q->new = 1;
	q->old = 1;

	query_run(q);

	i = 0;
	TAILQ_FOREACH(m, &q->results, next) {
		print_format(m);
		i++;
	}
	fprintf(stderr, "N%d\n", i);

	return 0;
}
