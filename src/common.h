#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

#define log(...) _log(__VA_ARGS__, "\n")
#define _log(FMT, ...)                                                         \
	fprintf(stderr, "%s:%s:%d " FMT "%s",                                  \
		strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1            \
				       : __FILE__,                             \
		__func__, __LINE__, __VA_ARGS__)

#define _elog(CODE, ...) (log(__VA_ARGS__), abort(), exit(CODE))

#define todo _elog(2, "UNIMPLEMENTED")
#define bug(...) _elog(127, "compiler bug: " __VA_ARGS__)
#define bug_if(...) ((__VA_ARGS__) ? bug("%s", #__VA_ARGS__) : 1)
#define bug_if_not(...) ((__VA_ARGS__) ? 1 : bug("not %s", #__VA_ARGS__))
#define error(...) _elog(1, "error: " __VA_ARGS__)
#define check(...) ((__VA_ARGS__) ? 1 : _elog(1, "check: not %s", #__VA_ARGS__))

#ifdef DEBUG
#define dbg(...) log("debug: " __VA_ARGS__)
#else
#define dbg(...)
#endif

#include "data.h"

#endif // COMMON_H
