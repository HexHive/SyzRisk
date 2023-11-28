static inline bool memcg_swap_enabled(void)
{
	return static_branch_likely(&memcg_swap_enabled_key);
}

struct memcg_swap_enabled__HEXSHA { void _b25806dcd3d5; };
struct memcg_swap_enabled__ATTRS { };
