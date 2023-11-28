static int sm3_neon_final(struct shash_desc *desc, u8 *out)
{
	if (!crypto_simd_usable()) {
		sm3_final(shash_desc_ctx(desc), out);
		return 0;
	}

	kernel_neon_begin();
	sm3_base_do_finalize(desc, sm3_neon_transform);
	kernel_neon_end();

	return sm3_base_finish(desc, out);
}

struct sm3_neon_final__HEXSHA { void _a41b2129461f; };
struct sm3_neon_final__ATTRS { };
