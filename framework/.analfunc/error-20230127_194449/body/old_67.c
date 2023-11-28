static int  setup_swap_account(char *s)
{
	bool res;

	if (!kstrtobool(s, &res))
		cgroup_memory_noswap = !res;
	return 1;
}

struct setup_swap_account__HEXSHA { void _b25806dcd3d5; };
struct setup_swap_account__ATTRS { void ___init; };
