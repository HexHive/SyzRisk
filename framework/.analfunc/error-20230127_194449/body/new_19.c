static int sm3_neon_update(struct shash_desc *desc, const u8 *data,
			   unsigned int len)
{
	if (!crypto_simd_usable()) {
		sm3_update(shash_desc_ctx(desc), data, len);
		return 0;
	}

	kernel_neon_begin();
	sm3_base_do_update(desc, data, len, sm3_neon_transform);
	kernel_neon_end();

	return 0;
}

struct sm3_neon_update__HEXSHA { void _a41b2129461f; };
struct sm3_neon_update__ATTRS { };
