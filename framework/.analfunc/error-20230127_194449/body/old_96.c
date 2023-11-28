static void kmalloc_memmove_invalid_size(struct kunit *test)
{
	char *ptr;
	size_t size = 64;
	volatile size_t invalid_size = size;

	ptr = kmalloc(size, GFP_KERNEL);
	KUNIT_ASSERT_NOT_ERR_OR_NULL(test, ptr);

	memset((char *)ptr, 0, 64);
	OPTIMIZER_HIDE_VAR(ptr);
	KUNIT_EXPECT_KASAN_FAIL(test,
		memmove((char *)ptr, (char *)ptr + 4, invalid_size));
	kfree(ptr);
}

struct kmalloc_memmove_invalid_size__HEXSHA { void _d6e5040bd8e5; };
struct kmalloc_memmove_invalid_size__ATTRS { };
