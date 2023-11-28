void kunit_kfree(struct kunit *test, const void *ptr)
{
	if (kunit_destroy_resource(test, kunit_kfree_match, (void *)ptr))
		KUNIT_FAIL(test, "kunit_kfree: %px already freed or not allocated by kunit", ptr);
}

struct kunit_kfree__HEXSHA { void _e562e309d1d4; };
struct kunit_kfree__ATTRS { };
