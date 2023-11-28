static  __alloc_size(3)
void *
{
	void *ret = kmem_cache_alloc(s, flags);

	ret = kasan_kmalloc(s, ret, size, flags);
	return ret;
}

struct __alloc_size__HEXSHA { void _eb4940d4adf5; };
struct __alloc_size__ATTRS { void ___always_inline; void ___alloc_size; };
