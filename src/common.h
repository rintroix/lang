#ifndef COMMON_H
#define COMMON_H

#define log(...) _log(__VA_ARGS__, "\n")
#define _log(FMT, ...)                                                         \
	printf("%s:%s:%d " FMT "%s",                                           \
	       strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__, \
	       __func__, __LINE__, __VA_ARGS__)

#define _elog(CODE, ...)                                                       \
	do {                                                                   \
		log(__VA_ARGS__);                                              \
		abort();                                                       \
		exit(CODE);                                                    \
	} while (0)

#define todo _elog(2, "UNIMPLEMENTED")
#define bug(...) _elog(127, "compiler bug: " __VA_ARGS__);
#define bug_if(...)                                                            \
	do {                                                                   \
		if (__VA_ARGS__)                                               \
			bug("%s", #__VA_ARGS__)                                \
	} while (0)
#define bug_if_not(...)                                                        \
	do {                                                                   \
		if (!(__VA_ARGS__))                                            \
			bug("not %s", #__VA_ARGS__)                            \
	} while (0)
#define error(...) _elog(1, "error: " __VA_ARGS__);
#define check(...)                                                             \
	do {                                                                   \
		if (!(__VA_ARGS__))                                            \
			_elog(1, "i checked, no good: %s", #__VA_ARGS__);      \
	} while (0)

#ifdef DEBUG
#  define dbg(...) log("debug: " __VA_ARGS__)
#else
#  define dbg(...)
#endif

#include "data.h"

#endif // COMMON_H
