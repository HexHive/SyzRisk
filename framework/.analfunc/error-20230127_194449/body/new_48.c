static void damon_test_set_regions(struct kunit *test)
{
	struct damon_target *t = damon_new_target();
	struct damon_region *r1 = damon_new_region(4, 16);
	struct damon_region *r2 = damon_new_region(24, 32);
	struct damon_addr_range range = {.start = 8, .end = 28};
	unsigned long expects[] = {8, 16, 16, 24, 24, 28};
	int expect_idx = 0;
	struct damon_region *r;

	damon_add_region(r1, t);
	damon_add_region(r2, t);
	damon_set_regions(t, &range, 1);

	KUNIT_EXPECT_EQ(test, damon_nr_regions(t), 3);
	damon_for_each_region(r, t) {
		KUNIT_EXPECT_EQ(test, r->ar.start, expects[expect_idx++]);
		KUNIT_EXPECT_EQ(test, r->ar.end, expects[expect_idx++]);
	}
	damon_destroy_target(t);
}

struct damon_test_set_regions__HEXSHA { void _62f409560eb2; };
struct damon_test_set_regions__ATTRS { };
