__FORTIFY_ ssize_t strscpy(char * const POS p, const char * const POS q, size_t size)
{
	size_t len;
	/* Use string size rather than possible enclosing struct size. */
	size_t p_size = __member_size(p);
	size_t q_size = __member_size(q);

	/* If we cannot get size of p and q default to call strscpy. */
	if (p_size == SIZE_MAX && q_size == SIZE_MAX)
		return __real_strscpy(p, q, size);

	/*
	 * If size can be known at compile time and is greater than
	 * p_size, generate a compile time write overflow error.
	 */
	if (__compiletime_lessthan(p_size, size))
		__write_overflow();

	/* Short-circuit for compile-time known-safe lengths. */
	if (__compiletime_lessthan(p_size, SIZE_MAX)) {
		len = __compiletime_strlen(q);

		if (len < SIZE_MAX && __compiletime_lessthan(len, size)) {
			__underlying_memcpy(p, q, len + 1);
			return len;
		}
	}

	/*
	 * This call protects from read overflow, because len will default to q
	 * length if it smaller than size.
	 */
	len = strnlen(q, size);
	/*
	 * If len equals size, we will copy only size bytes which leads to
	 * -E2BIG being returned.
	 * Otherwise we will copy len + 1 because of the final '\O'.
	 */
	len = len == size ? size : len + 1;

	/*
	 * Generate a runtime write overflow error if len is greater than
	 * p_size.
	 */
	if (len > p_size)
		fortify_panic(__func__);

	/*
	 * We can now safely call vanilla strscpy because we are protected from:
	 * 1. Read overflow thanks to call to strnlen().
	 * 2. Write overflow thanks to above ifs.
	 */
	return __real_strscpy(p, q, len);
}

struct strscpy__HEXSHA { void _62e1cbfc5d79; };
struct strscpy__ATTRS { void _INLINE; };
