#ifndef UM_STR_H
#define UM_STR_H

#include "um/deq.h"
#include <stdarg.h>
#include <stdio.h>

static inline umd(char) umd_dup_exact(const char *restrict s)
{
	size_t len = strlen(s);
	umd(char) out = umd_new_manual(char, len);
	memcpy(UmDBucket(out)->data, s, len);
	UmDHead(out)->len = len;
	UmDBucket(out)->end = len;
	return out;
}

static inline int umd_append_vfmt(umd(char) xs, const char *restrict fmt,
				  va_list base)
{
	va_list clone;
	va_copy(clone, base);

	_umd_h *h = UmDGHead(xs);
	size_t cap = h->cap;
	size_t one = sizeof(char);
	_umd_b *b = UmDLastBucket(UmDGBucket(xs), 2, cap, one);
	size_t len = cap - b->end;

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
	va_end(clone);

	return written;
}

static inline int umd_append_fmt(umd(char) xs, const char *restrict fmt, ...)
{
	va_list base;
	va_start(base, fmt);
	int ret = umd_append_vfmt(xs, fmt, base);
	va_end(base);

	return ret;
}

static inline int umd_cmp(umd(char) xs, const char *restrict s)
{
	umd_each(xs, c, i)
	{
		if (c != *s)
			return c > *s ? 1 : -1;
		s++;
	}

	return *s == '\0' ? 0 : -1;
}

static inline char *umd_to_cstr(umd(char) xs)
{
	char *out = malloc(umd_len(xs) + 1);
	umd_each(xs, c, i) { out[i] = c; }
	out[umd_len(xs)] = '\0';
	return out;
}

#endif // UM_STR_H
