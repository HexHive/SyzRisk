static void  sm3_neon_fini(void)
{
	crypto_unregister_shash(&sm3_alg);
}

struct sm3_neon_fini__HEXSHA { void _a41b2129461f; };
struct sm3_neon_fini__ATTRS { void ___exit; };
