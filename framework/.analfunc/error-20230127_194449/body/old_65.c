static bool do_memsw_account(void)
{
	return !cgroup_subsys_on_dfl(memory_cgrp_subsys) && memcg_swap_enabled();
}

struct do_memsw_account__HEXSHA { void _b25806dcd3d5; };
struct do_memsw_account__ATTRS { };
