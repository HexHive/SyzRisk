static int sm3_neon_finup(struct shash_desc *desc, const u8 *data,
			  unsigned int len, u8 *out)
{
	if (!crypto_simd_usable()) {
		struct sm3_state *sctx = shash_desc_ctx(desc);

		if (len)
			sm3_update(sctx, data, len);
		sm3_final(sctx, out);
		return 0;
	}

	kernel_neon_begin();
	if (len)
		sm3_base_do_update(desc, data, len, sm3_neon_transform);
	sm3_base_do_finalize(desc, sm3_neon_transform);
	kernel_neon_end();

	return sm3_base_finish(desc, out);
}

struct sm3_neon_finup__HEXSHA { void _a41b2129461f; };
struct sm3_neon_finup__ATTRS { };
