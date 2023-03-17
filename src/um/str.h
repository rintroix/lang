#ifndef UM_STR_H
#define UM_STR_H

#include "um/deq.h"
#include <stdio.h>
#include <stdarg.h>

#define ums umd(char)
#define ums_len(S) umd_len(S)
#define ums_new() umd_new(char)
#define ums_new_manual(N) umd_new_manual(char, N)

static inline ums ums_dup_exact(char* s)  {
	size_t len = strlen(s);
	ums out = ums_new_manual(len);
	memcpy(UmDBucket(out)->data, s, len); 
	assert(UmDHead(out)->cap == len);
	UmDHead(out)->len = len;
	UmDBucket(out)->end = len;
	return out;
}

static inline int ums_append_fmt(ums xs, const char *restrict fmt, ...) {
	va_list base;
	va_list clone;

	_umd_head(char) * h = (void*)UmDHead(xs);
	_umd_bucket(char) * b = (void*)UmDBucket(xs);

	size_t cap = h->cap;
	size_t one = sizeof(char);

	UmDNextSpaceOnEnd(b, 2, cap, one);

	size_t len = cap - b->end;

	va_start(base, fmt);
	va_copy(clone, base);

	int written = vsnprintf(((char *)b->data) + b->end, len, fmt, base);

	if (written < 0)
		goto cleanup;

	if (written + 1 > len) {
 		if (written + 1 > cap) {
			assert(0 && "fixme: need bigger buckets");
		}

		UmDAddBucket(b, cap, one);

		written = vsnprintf(((char *)b->data), cap, fmt, clone);

		if (written < 0)
			goto cleanup;
	}

	b->end += written;
	h->len += written;

cleanup:
	va_end(base);
	va_end(clone);

	return written;
}

static inline int ums_cmp(ums xs, const char *restrict s) {
	umd_each(xs, c, i) {
		if (c != *s)
			return c > *s ? 1 : -1;
		s++;
	}

	return *s == '\0' ? 0 : -1;
}

static inline char* ums_to_cstr(ums xs) {
	char *out = malloc(umd_len(xs) + 1);
	umd_each(xs, c, i) {
		out[i] = c;
	}
	out[umd_len(xs)] = '\0';
	return out;
}

#endif // UM_STR_H

