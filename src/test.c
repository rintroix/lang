#include "greatest.h"
#include "um/um.h"

TEST vecs(void)
{
	umv(int) v = umv_new_manual(int, 2);

	ASSERT(umv_len(v) == 0);
	ASSERT(UmVHead(v)->cap == 2);
	ASSERT(UmVHead(v)->bucket.next == (void*)0);
	ASSERT(UmVCountBuckets(v) == 1);

	umv_push(v, 1);
	umv_push(v, 2);

	ASSERT(umv_len(v) == 2);
	ASSERT(UmVHead(v)->bucket.next == (void*)0);
	ASSERT(UmVCountBuckets(v) == 1);

	umv_push(v, 3);
	umv_push(v, 4);

	ASSERT(umv_len(v) == 4);
	ASSERT(UmVCountBuckets(v) == 2);

	umv_push(v, 5);
	umv_push(v, 6);

	ASSERT(umv_len(v) == 6);
	ASSERT(UmVCountBuckets(v) == 3);

	ASSERT(*umv_at(v, 0) == 1);
	ASSERT(*umv_at(v, 1) == 2);
	ASSERT(*umv_at(v, 2) == 3);
	ASSERT(umv_get(v, 3) == 4);
	ASSERT(umv_get(v, 4) == 5);
	ASSERT(umv_get(v, 5) == 6);

	int sum = 0;
	umv_each(v, n) { sum += n; }
	ASSERT(sum == 21);

	sum = 0;
	umv_loop(v, n, 3, 6) { sum += n; }
	ASSERT(sum == 15);

	umv(int) slice1 = umv_slice(v, 2, 5);

	sum = 0;
	umv_each(slice1, n) { sum += n; }

	ASSERT(sum == 12);
	ASSERT(umv_len(slice1) == 3);

	PASS();
}

TEST empty_vec(void)
{
	umv(int) a = umv_new(int);
	ASSERT(umv_len(a) == 0);

	umv(int) b = umv_slice(a, 0, umv_len(a));
	ASSERT(umv_len(b) == 0);

	int sum = 0;
	umv_each(a, n) {
		sum += n;
	}
	ASSERT(sum == 0);

	umv_loop(a, n, 0, umv_len(a)) {
		sum += n;
	}
	ASSERT(sum == 0);

	PASS();
}

TEST deqs(void)
{
	umd(int) a = umd_new_manual(int, 1);

	ASSERT(umd_len(a) == 0);

	ASSERT(0 == umd_push(a, 100));
	ASSERT(1 == umd_push(a, 200));
	ASSERT(2 == umd_push(a, 300));
	ASSERT(3 == umd_push(a, 400));
	ASSERT(4 == umd_push(a, 500));

	ASSERT(umd_len(a) == 5);

	int suma = 0;
	umd_each(a, n) {
		suma += n;	
	}

	ASSERT(suma == 500 + 400 + 300 + 200 + 100);

	ASSERT(umd_get(a, 0) == 100);
	ASSERT(umd_get(a, 1) == 200);
	ASSERT(umd_get(a, 2) == 300);
	ASSERT(umd_get(a, 3) == 400);
	ASSERT(umd_get(a, 4) == 500);

	umd(int*) b = umd_new_manual(int*, 2);
	int i1 = 10, i2 = 20, i3 = 30;

	ASSERT(0 == umd_push(b, &i1));
	ASSERT(1 == umd_push(b, &i2));
	ASSERT(2 == umd_push(b, &i3));

	ASSERT(umd_get(b, 0) == &i1);
	ASSERT(umd_get(b, 1) == &i2);
	ASSERT(umd_get(b, 2) == &i3);

	int sumb = 0;
	umd_each(b, n) {
		sumb += *n;	
	}

	ASSERT(sumb == 10 + 20 + 30);

	PASS();
}

TEST strs(void) {
	char *init = "what's up, hello";
	umd(char) a = umd_dup_exact(init);

	ASSERT(0 == strncmp(UmDBucket(a)->data, init, strlen(init)));

	umd_append_fmt(a, " world %d", 10);

	char *result = "what's up, hello world 10";
	ASSERT(0 == umd_cmp(a, result));

	umd(char) s1 = umd_dup_exact("hi");

	ASSERT( 0 == umd_cmp(s1, "hi"));
	ASSERT(-1 == umd_cmp(s1, "hit"));
	ASSERT( 1 == umd_cmp(s1, "ha"));

	umd_append_fmt(s1, "X");
	umd_append_fmt(s1, "Y");
	umd_append_fmt(s1, "X");

	ASSERT(0 == umd_cmp(s1, "hiXYX"));
	ASSERT(UmDCountBuckets(s1) == 4);

	umd(char) s2 = umd_dup_exact("hello  ");
	umd_append_fmt(s2, "world"); 
	umd_append_fmt(s2, "!!!");

	ASSERT(0 == umd_cmp(s2, "hello  world!!!"));
	ASSERT(UmDCountBuckets(s2) == 3);
 
	PASS();
}

/* Suites can group multiple tests with common setup. */
SUITE(vec_suite)
{
	RUN_TEST(vecs);
	RUN_TEST(deqs);
	RUN_TEST(strs);
	RUN_TEST(empty_vec);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv)
{
	GREATEST_MAIN_BEGIN(); /* command-line options, initialization. */
	RUN_SUITE(vec_suite);
	GREATEST_MAIN_END(); /* display results */
}
