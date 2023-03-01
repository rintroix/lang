#include "greatest.h"
#include "um.h"

/* A test runs various assertions, then calls PASS(), FAIL(), or SKIP(). */
TEST vecs(void) {
	um_vec(int) v = (um_vec(int))_um_vec_alloc(2 * sizeof(int), 0);

	um_vec_head_t *head = um_vec_head(v);

	ASSERT(um_vec_len(v) == 0);
	ASSERT(um_vec_head(v)->alloc);
	ASSERT(!um_vec_head(v)->next);

	um_vec_push(v, 1);
	um_vec_push(v, 2);

	ASSERT_EQ(um_vec_head(v)->count, 2);
	ASSERT(!um_vec_head(v)->next);

	um_vec_push(v, 3);
	um_vec_push(v, 4);

	ASSERT_EQ(um_vec_head(v)->count, 2);
	ASSERT_EQ(um_vec_head(v)->next->count, 2);
	ASSERT(um_vec_head(v)->next);
	ASSERT(! um_vec_head(v)->next->next);

	um_vec_push(v, 5);
	um_vec_push(v, 6);

	ASSERT(*um_vec_at(v, 0) == 1);
	ASSERT(*um_vec_at(v, 1) == 2);
	ASSERT(*um_vec_at(v, 2) == 3);
	ASSERT(*um_vec_at(v, 3) == 4);
	ASSERT(*um_vec_at(v, 4) == 5);
	ASSERT(*um_vec_at(v, 5) == 6);

	ASSERT(um_vec_head(v)->next->next);
	ASSERT(!um_vec_head(v)->next->next->next);

	int sum = 0;
	um_vec_for(v, it) {
		sum += *it.value;
	}

	ASSERT(sum == 21);

	um_vec(int) slice1 = um_vec_slice(v, 2, 5);

	sum = 0;
	um_vec_for(slice1, it) {
		sum += *it.value;
	}

	ASSERT(sum == 12);

	PASS();
}

void foo(int *i) {
	
}

TEST anon_structs(void) {
	PASS();
}

/* Suites can group multiple tests with common setup. */
SUITE(vec_suite) {
    RUN_TEST(vecs);
    RUN_TEST(anon_structs);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();      /* command-line options, initialization. */
    RUN_SUITE(vec_suite);
    GREATEST_MAIN_END();        /* display results */
}
