#include "greatest.h"
#include "um/um.h"

/* A test runs various assertions, then calls PASS(), FAIL(), or SKIP(). */
TEST vecs(void)
{
	um_vec(int) v = um_vec_alloc_manual(v, 2);

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

void foo(int *i) {}

TEST anon_structs(void) { PASS(); }

/* Suites can group multiple tests with common setup. */
SUITE(vec_suite)
{
	RUN_TEST(vecs);
	RUN_TEST(anon_structs); // TODO
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv)
{
	GREATEST_MAIN_BEGIN(); /* command-line options, initialization. */
	RUN_SUITE(vec_suite);
	GREATEST_MAIN_END(); /* display results */
}
