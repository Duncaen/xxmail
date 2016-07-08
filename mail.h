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

struct query {
	int hdr;
	int msgbox;
	int new;
	int old;
	TAILQ_HEAD(, mailbox) boxes;
	TAILQ_HEAD(, header_cb) header_cb;
	TAILQ_HEAD(, result_cb) result_cb;
};

struct mailbox {
	TAILQ_ENTRY(mailbox) next;
	int (*query)(struct mailbox *mb, struct query *q);
	void *arg;
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

/**/
typedef int (*header_cb_func)(struct query *q, struct mail *m,
    char *hdr, void *arg);

/**/
typedef void (*result_cb_func)(struct query *q, struct mail *m,
    void *arg);

struct header_cb {
	TAILQ_ENTRY(header_cb) next;
	header_cb_func func;
	void *arg;
};

struct result_cb {
	TAILQ_ENTRY(result_cb) next;
	result_cb_func func;
	void *arg;
};

/* compat */
size_t strlcpy(char *dst, const char *src, size_t dsize);
size_t strlcat(char *dst, const char *src, size_t dsize);

/* query */
void query_add_mailbox(struct query *q, struct mailbox *mb);
void query_add_header_cb(struct query *q, header_cb_func f);
void query_add_result_cb(struct query *q, result_cb_func f);
int query_header_cb(struct query *q, struct mail *m, char *hdr);
int query_result_cb(struct query *q, struct mail *m);
int query_run(struct query *q);
struct query * query_init();
void query_free(struct query *q);

/* maildir */
int maildir_query(struct mailbox *mb, struct query *q);
struct mailbox * maildir_init(const char *path);

/* mail */
int mail_mmap(struct mail *m, const char *path);
int mail_stream(struct mail *m, const char *path);
struct mail * mail_init(struct query *q);

#endif
