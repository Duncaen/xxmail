#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "sys-queue.h"

#include "mail.h"

void
query_add_mailbox(struct query *q, struct mailbox *mb)
{
	TAILQ_INSERT_TAIL(&q->boxes, mb, next);
}

void
query_add_header_cb(struct query *q, header_cb_func f)
{
	struct header_cb *cb;

	cb = calloc(1, sizeof(*cb));
	if (!cb)
		err(1, "calloc");

	cb->func = f;

	TAILQ_INSERT_TAIL(&q->header_cb, cb, next);
}

void
query_add_result_cb(struct query *q, result_cb_func f)
{
	struct result_cb *cb;

	cb = calloc(1, sizeof(*cb));
	if (!cb)
		err(1, "calloc");

	cb->func = f;

	TAILQ_INSERT_TAIL(&q->result_cb, cb, next);
}

int
query_header_cb(struct query *q, struct mail *m, char *hdr)
{
	struct header_cb *cb;

	TAILQ_FOREACH(cb, &q->header_cb, next) {
		if (cb->func == NULL)
			continue;
		if (cb->func(q, m, hdr, cb->arg) == -1)
			return -1;
	}

	return 0;
}

int
query_result_cb(struct query *q, struct mail *m)
{
	struct result_cb *cb;

	TAILQ_FOREACH(cb, &q->result_cb, next) {
		if (cb->func == NULL)
			continue;
		cb->func(q, m, cb->arg);
	}

	return 0;
}

int
query_run(struct query *q)
{
	struct mailbox *mb;
	TAILQ_FOREACH(mb, &q->boxes, next) {
		mb->query(mb, q);
	}
	return 0;
}


struct query *
query_init()
{
	struct query *q;

	q = calloc(1, sizeof(*q));
	if (!q)
		err(1, "calloc");

	TAILQ_INIT(&q->boxes);
	TAILQ_INIT(&q->header_cb);
	TAILQ_INIT(&q->result_cb);

	return q;
}

int
query_reset()
{
	return 0;
}

void
query_free(struct query *q)
{
	struct header_cb *hcb;
	struct result_cb *rcb;

	TAILQ_FOREACH(hcb, &q->header_cb, next) {
		TAILQ_REMOVE(&q->header_cb, hcb, next);
		free(hcb);
	}

	TAILQ_FOREACH(rcb, &q->result_cb, next) {
		TAILQ_REMOVE(&q->result_cb, rcb, next);
		free(rcb);
	}
}
