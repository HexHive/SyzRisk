void kunit_kfree(struct kunit *test, const void *ptr)
{
	struct kunit_resource *res;

	res = kunit_find_resource(test, kunit_kfree_match, (void *)ptr);

	/*
	 * Removing the resource from the list of resources drops the
	 * reference count to 1; the final put will trigger the free.
	 */
	kunit_remove_resource(test, res);

	kunit_put_resource(res);

}

struct kunit_kfree__HEXSHA { void _e562e309d1d4; };
struct kunit_kfree__ATTRS { };
