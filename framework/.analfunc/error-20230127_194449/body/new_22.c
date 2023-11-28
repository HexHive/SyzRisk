static int  sm3_neon_init(void)
{
	return crypto_register_shash(&sm3_alg);
}

struct sm3_neon_init__HEXSHA { void _a41b2129461f; };
struct sm3_neon_init__ATTRS { void ___init; };
