#ifndef _MAIL_H
#define _MAIL_H

#include <stdint.h>

#include "sys-queue.h"
#include "sys-tree.h"

enum hdr {
	HDR_FROM = 1,
	HDR_SUBJECT,
	HDR_MESSAGE_ID,
	HDR_IN_REPLY_TO,
};

struct result {
	int *hdrs;
	struct query *q;
	TAILQ_HEAD(, mail) mails;
};

struct query {
	int hdr;
	int msgbox;
	int new;
	int old;
	TAILQ_HEAD(, mailbox) boxes;
	TAILQ_HEAD(, mail) results;
	struct regex_t *preg;
};

struct mailbox {
	TAILQ_ENTRY(mailbox) next;
	int (*query)(struct mailbox *mb, struct query *q);
	void *data;
	char *path;
};

struct mail {
	TAILQ_ENTRY(mail) next;
	int idx;
	int state;
	char *subject;
	char *from;
	struct query *query;
};

struct maildir {
	char *path;
	int fdnew;
	int fdcur;
};

/* compat */
size_t strlcpy(char *dst, const char *src, size_t dsize);
size_t strlcat(char *dst, const char *src, size_t dsize);

/* query */
int query_run(struct query *q);
void query_add_mailbox(struct query *q, struct mailbox *mb);
void query_add_result(struct query *q, struct mail *m);
struct query * query_init();

/* maildir */
int maildir_query(struct mailbox *mb, struct query *q);
struct mailbox * maildir_init(const char *path);

/* mail */
int mail_mmap(struct mail *m, const char *path);
int mail_stream(struct mail *m, const char *path);
struct mail * mail_init(struct query *q);

#endif
