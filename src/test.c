#include "greatest.h"
#include "data.h"

um_vec_type(int);

/* A test runs various assertions, then calls PASS(), FAIL(), or SKIP(). */
TEST vec_init(void) {
	um_vec(int) *v = um_vec_alloc(int, 2);

	ASSERT(v->count == 0);
	ASSERT(v->max == 2);
	ASSERT(v->alloc);
	ASSERT(! v->next);

	um_vec_push(v, 1);
	um_vec_push(v, 2);

	ASSERT(! v->next);

	um_vec_push(v, 3);
	um_vec_push(v, 4);

	ASSERT(v->next);
	ASSERT(! v->next->next);

	ASSERT(v->next->data[0] == 3);
	ASSERT(v->next->data[1] == 4);

	PASS();
}

/* Suites can group multiple tests with common setup. */
SUITE(vec_suite) {
    RUN_TEST(vec_init);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
    GREATEST_MAIN_BEGIN();      /* command-line options, initialization. */
    RUN_SUITE(vec_suite);
    GREATEST_MAIN_END();        /* display results */
}
