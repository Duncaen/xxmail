#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <err.h>

#include <sys/stat.h>

#include "mail.h"

static char *argv0;
static char *format;

static char default_format[] = "%i [%F] [%S]\n";

static TAILQ_HEAD(, mail) results;
static int num_result = 0;


static void
usage()
{
	fprintf(stderr, "Usage: %s [-f FMT] PATH...\n", argv0);
	exit(2);
}

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

void
lazy_result_cb(struct query *q, struct mail *m, void *arg)
{
	TAILQ_INSERT_TAIL(&results, m, next);
}

void
print_result_cb(struct query *q, struct mail *m, void *arg)
{
	print_format(m);
	num_result++;
}

int
main(int argc, char *argv[])
{
	int c, i, lazy;
	struct stat st;
	struct query *q;
	struct mailbox *mb;
	struct mail *m;

	argv0 = argv[0];
	format = default_format;
	lazy = 0;

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

	if (lazy) {
		TAILQ_INIT(&results);
		query_add_result_cb(q, lazy_result_cb);
	} else
		query_add_result_cb(q, print_result_cb);

	query_run(q);

	if (lazy) {
		TAILQ_FOREACH(m, &results, next) {
			print_format(m);
			num_result++;
		}
	}

	fprintf(stderr, "N%d\n", num_result);

	return 0;
}
