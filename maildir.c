#define _GNU_SOURCE
#include <dirent.h>     /* Defines DT_* constants */
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <err.h>

#include "mail.h"

struct linux_dirent64 {
	ino64_t        d_ino;    /* 64-bit inode number */
	off64_t        d_off;    /* 64-bit offset to next structure */
	unsigned short d_reclen; /* Size of this dirent */
	unsigned char  d_type;   /* File type */
	char           d_name[]; /* Filename (null-terminated) */
};

#define BUF_SIZE 1024000

int
maildir_read(struct mailbox *mb, struct query *q, const char *dir)
{
	struct mail *m;
	struct linux_dirent64 *d;
	int fd, bpos, nread;
	char buf[BUF_SIZE];

	if (chdir(dir) == -1)
		err(1, "chdir");

	if ((fd = open(".", O_RDONLY | O_DIRECTORY)) < 0)
		err(1, "open");

	while (1) {
		nread = syscall(SYS_getdents64, fd, buf, BUF_SIZE);
		if (nread < 0)
			err(1, "SYS_getdents64");
		if (nread == 0)
			break;

		for (bpos  = 0; bpos < nread;) {
			d = (struct linux_dirent64 *) (buf + bpos);
			if (d->d_type != DT_REG)
				goto next;
			if (d->d_name[0] == '.')
				goto next;

			char *flags = strstr(d->d_name, ":2,");
			if (!flags)
				goto next;

			m = mail_init(q);
			mail_mmap(m, d->d_name);
			query_add_result(q, m);

next:
			bpos += d->d_reclen;
		}
	}

	close(fd);

	if (chdir("..") == -1)
		err(1, "chdir");

	return 0;
}

int
maildir_query(struct mailbox *mb, struct query *q)
{
	if (chdir(mb->path) == -1)
		err(1, "Failed to open maildir %s", mb->path);

	if (q->new && maildir_read(mb, q, "new") == -1)
		err(1, "Failed to read %s/new", mb->path);

	if (q->old && maildir_read(mb, q, "cur") == -1)
		err(1, "Failed to read %s/cur", mb->path);

	return 0;
}

struct mailbox *
maildir_init(const char *path)
{
	struct mailbox *mb;

	if (!path)
		err(1, "Failed to open maildir");

	mb = calloc(1, sizeof(*mb));
	if (!mb)
		return NULL;

	mb->query = &maildir_query;
	mb->path = strdup(path);

	return mb;
}
