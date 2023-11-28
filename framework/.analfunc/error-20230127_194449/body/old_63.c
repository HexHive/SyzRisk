static int  mem_cgroup_swap_init(void)
{
	/* No memory control -> no swap control */
	if (mem_cgroup_disabled())
		cgroup_memory_noswap = true;

	if (cgroup_memory_noswap)
		return 0;

	static_branch_enable(&memcg_swap_enabled_key);

	WARN_ON(cgroup_add_dfl_cftypes(&memory_cgrp_subsys, swap_files));
	WARN_ON(cgroup_add_legacy_cftypes(&memory_cgrp_subsys, memsw_files));

	WARN_ON(cgroup_add_dfl_cftypes(&memory_cgrp_subsys, zswap_files));

	return 0;
}

struct mem_cgroup_swap_init__HEXSHA { void _b25806dcd3d5; };
struct mem_cgroup_swap_init__ATTRS { void ___init; };
