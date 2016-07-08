#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <err.h>

#include <sys/mman.h>
#include <sys/stat.h>

#include "mail.h"

#define	RFC2822_MAX_LINE_SIZE	4096

static int
mail_parse_from(struct mail *m, const char *b)
{
	return (m->from = strdup(b)) != NULL;
}

static int
mail_parse_subject(struct mail *m, const char *b)
{
	return (m->subject = strdup(b)) != NULL;
}

static int
mail_parse_date(struct mail *m, const char *b)
{
	return 0;
}

static int
mail_parse_msgid(struct mail *m, const char *b)
{
	return 0;
}

static int
mail_parse_inreplyto(struct mail *m, const char *b)
{
	return 0;
}

static int
mail_parse_hdr(struct mail *m, char *b)
{
	int l = strlen(b);
	if (l > 4 && b[4] == ':') {
		if (b[0] == 'f' && b[1] == 'r' &&
				b[2] == 'o' && b[3] == 'm') {
			/* from: */
			return mail_parse_from(m, b);
		} else if (b[0] == 'D' && b[1] == 'a' &&
				b[2] == 't' && b[3] == 'e') {
			/* date: */
			return mail_parse_date(m, b);
		}
	} else if (l > 7 && b[7] == ':') {
		if (b[0] == 's' && b[1] == 'u' &&
				b[2] == 'b' && b[3] == 'j' &&
				b[4] == 'e' && b[5] == 'c' &&
				b[6] == 't') {
			/* subject: */
			return mail_parse_subject(m, b);
		}
	} else if (l > 10 && b[10] == ':') {
		if (b[0] == 'm' && b[1] == 'e' &&
				b[2] == 's' && b[3] == 's' &&
				b[4] == 'a' && b[5] == 'g' &&
				b[6] == 'e' && b[7] == '-' &&
				b[8] == 'i' && b[9] == 'd') {
			/* message-id: */
			return mail_parse_msgid(m, b);
		}
	} else if (l > 11 && b[11] == ':') {
		if (b[0] == 'i' && b[1] == 'n' &&
				b[2] == '-' && b[3] == 'r' &&
				b[4] == 'e' && b[5] == 'p' &&
				b[6] == 'l' && b[7] == 'y' &&
				b[8] == '-' && b[9] == 't' &&
				b[10] == 'o') {
			/* in-reply-to: */
			return mail_parse_inreplyto(m, b);
		}
	}
	return 0;
}

static size_t
mail_mmnormalize(char *start, size_t len)
{
	char *b, *nl, *col, *end;
	int fold = 0;
	b = start;
	end = b + len;

	while(b + 1 < end) {
		/* find next newline */
		nl = memchr(b, '\n', (end - b));
		if (!nl || nl + 1 >= end)
			break;

		/* if not inside of header fold the header field */
		if (!fold) {
			col = memchr(b, ':', (nl - b));
			if (col)
				for (int i = 0; i < (col - b); i++)
					b[i] = tolower(b[i]);
		}

		/* \nWSP == folded header */
		if (!(fold = (nl[1] == ' ' || nl[1] == '\t')))
			nl[0] = '\0';

		/* \n\n == body reached */
		if (nl[1] == '\n') {
			nl[1] = '\0';
			break;
		}

		b = nl + 1;
	}

	return (nl - start);
}

int
mail_mmap(struct mail *m, const char *path)
{
	char *b;
	int fd;
	struct stat st;

	if ((fd = open(path, O_RDONLY)) < 0)
		err(1, "open");

	if (fstat(fd, &st) == -1)
		err(1, "stat");

	b = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if (b == (void *)-1)
		err(1, "mmap");

	madvise(b, st.st_size, MADV_SEQUENTIAL);
	
	mail_mmnormalize(b, (size_t) st.st_size);

	/*
	char *needle = " from:";
	size_t needle_len = strlen(needle) + 1;
	needle[0] = '\0';

	printf("nlen %ld\n", needle_len);
	char *from = (char*) memmem((void*) b, hlen, needle, needle_len);
	if (from) {
		int diff = from - b;
		from = b + diff;
		printf("FROM: %s\n%s\n", from, path);
	}
	*/

	int in_hdr = 1;
	char *r = b;
	while (r < (b + st.st_size)) {
		if (r[0] == '\0') {
			in_hdr = 0;
			r = r + 1;
			continue;
		}
		if (in_hdr)
			mail_parse_hdr(m, r);
		/*
		else
			printf("LN: '%c' '%d' '%s'\n", r[0], r == '\0', r);
		*/
		r = r + strlen(r) + 1;
	}

	munmap(b, st.st_size);
	close(fd);
	return 0;
}

int
mail_stream(struct mail *m, const char *path)
{
	/*
	FILE *fp;
	fp = fopen(d->d_name, "r");
	if (fp == NULL)
		err(1, "fopen");
	fclose(fp);
	*/
	return 0;
}

struct mail *
mail_init(struct query *q)
{
	struct mail *m;

	m = calloc(1, sizeof(*m));
	if (!m)
		err(1, "calloc");

	m->query = q;
	return m;
}
