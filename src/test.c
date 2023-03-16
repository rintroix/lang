#include "greatest.h"
#include "um/um.h"

/* A test runs various assertions, then calls PASS(), FAIL(), or SKIP(). */
TEST vecs(void)
{
	um_vec(int) v = um_vec_alloc_manual(int, 2);

	um_vec_h *head = UmVHead(v);

	ASSERT(um_vec_len(v) == 0);
	ASSERT(head->alloc);
	ASSERT(!head->next);

	um_vec_push(v, 1);
	um_vec_push(v, 2);

	ASSERT_EQ(head->count, 2);
	ASSERT(!head->next);

	um_vec_push(v, 3);
	um_vec_push(v, 4);

	ASSERT_EQ(head->count, 2);
	ASSERT_EQ(head->next->count, 2);
	ASSERT(head->next);
	ASSERT(!head->next->next);

	um_vec_push(v, 5);
	um_vec_push(v, 6);

	ASSERT(um_vec_len(v) == 6);

	ASSERT(*um_vec_at(v, 0) == 1);
	ASSERT(*um_vec_at(v, 1) == 2);
	ASSERT(*um_vec_at(v, 2) == 3);
	ASSERT(um_vec_get(v, 3) == 4);
	ASSERT(um_vec_get(v, 4) == 5);
	ASSERT(um_vec_get(v, 5) == 6);

	ASSERT(head->next->next);
	ASSERT(!head->next->next->next);

	int sum = 0;
	um_vec_for(v, it, i) { sum += *it; }
	ASSERT(sum == 21);

	sum = 0;
	um_vec_for_range(v, it, 3, 6) { sum += *it; }
	ASSERT(sum == 15);

	um_vec(int) slice1 = um_vec_slice(v, 2, 5);

	sum = 0;
	um_vec_for(slice1, it) { sum += *it; }

	ASSERT(sum == 12);
	ASSERT(um_vec_len(slice1) == 3);

	PASS();
}

TEST empty_vec(void)
{
	um_vec(int) a = um_vec_alloc(int);
	ASSERT(um_vec_len(a) == 0);

	um_vec(int) b = um_vec_slice(a, 0, um_vec_len(a));
	ASSERT(um_vec_len(b) == 0);

	int sum = 0;
	um_vec_for(a, np) {
		sum += *np;
	}
	ASSERT(sum == 0);

	um_vec_for_range(a, np, 0, um_vec_len(a)) {
		sum += *np;
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
	umd_for(a, n) {
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
	umd_for(b, n) {
		sumb += *n;	
	}

	ASSERT(sumb == 10 + 20 + 30);

	PASS();
}

TEST strs(void) {
	char *init = "what's up, hello";
	ums a = ums_dup(init);

	ASSERT(0 == strncmp(UmDBucket(a)->data, init, strlen(init)));

	ums_append_fmt(a, " world %d", 10);

	char *result = "what's up, hello world 10";
	ASSERT(0 == ums_cmp(a, result));

	ums s1 = ums_dup("hi");

	ASSERT( 0 == ums_cmp(s1, "hi"));
	ASSERT(-1 == ums_cmp(s1, "hit"));
	ASSERT( 1 == ums_cmp(s1, "ha"));

	ums_append_fmt(s1, "X");
	ums_append_fmt(s1, "Y");
	ums_append_fmt(s1, "X");

	ASSERT(0 == ums_cmp(s1, "hiXYX"));
	ASSERT(UmDCountBuckets(s1) == 4);

	ums s2 = ums_dup("hello  ");
	ums_append_fmt(s2, "world"); 
	ums_append_fmt(s2, "!!!");

	ASSERT(0 == ums_cmp(s2, "hello  world!!!"));
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
