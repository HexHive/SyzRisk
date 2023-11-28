static void krealloc_more_oob_helper(struct kunit *test,
					size_t size1, size_t size2)
{
	char *ptr1, *ptr2;
	size_t middle;

	KUNIT_ASSERT_LT(test, size1, size2);
	middle = size1 + (size2 - size1) / 2;

	ptr1 = kmalloc(size1, GFP_KERNEL);
	KUNIT_ASSERT_NOT_ERR_OR_NULL(test, ptr1);

	ptr2 = krealloc(ptr1, size2, GFP_KERNEL);
	KUNIT_ASSERT_NOT_ERR_OR_NULL(test, ptr2);

	/* Suppress -Warray-bounds warnings. */
	OPTIMIZER_HIDE_VAR(ptr2);

	/* All offsets up to size2 must be accessible. */
	ptr2[size1 - 1] = 'x';
	ptr2[size1] = 'x';
	ptr2[middle] = 'x';
	ptr2[size2 - 1] = 'x';

	/* Generic mode is precise, so unaligned size2 must be inaccessible. */
	if (IS_ENABLED(CONFIG_KASAN_GENERIC))
		KUNIT_EXPECT_KASAN_FAIL(test, ptr2[size2] = 'x');

	/* For all modes first aligned offset after size2 must be inaccessible. */
	KUNIT_EXPECT_KASAN_FAIL(test,
		ptr2[round_up(size2, KASAN_GRANULE_SIZE)] = 'x');

	kfree(ptr2);
}

struct krealloc_more_oob_helper__HEXSHA { void _d6e5040bd8e5; };
struct krealloc_more_oob_helper__ATTRS { };
