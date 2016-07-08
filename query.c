#include <stdlib.h>
#include <string.h>
#include <err.h>

#include "sys-queue.h"

#include "mail.h"

int
query_run(struct query *q)
{
	struct mailbox *mb;
	TAILQ_FOREACH(mb, &q->boxes, next) {
		mb->query(mb, q);
	}
	return 0;
}

void
query_add_result(struct query *q, struct mail *m)
{
	TAILQ_INSERT_TAIL(&q->results, m, next);
}

void
query_add_mailbox(struct query *q, struct mailbox *mb)
{
	TAILQ_INSERT_TAIL(&q->boxes, mb, next);
}

struct query *
query_init()
{
	struct query *q;
	q = calloc(1, sizeof(*q));
	if (!q)
		err(1, "calloc query");
	TAILQ_INIT(&q->boxes);
	TAILQ_INIT(&q->results);
	return q;
}

int
query_reset()
{
	return 0;
}

int
query_free(struct query *q)
{
	return 0;
}
