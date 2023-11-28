bool mem_cgroup_swap_full(struct folio *folio)
{
	struct mem_cgroup *memcg;

	VM_BUG_ON_FOLIO(!folio_test_locked(folio), folio);

	if (vm_swap_full())
		return true;
	if (!memcg_swap_enabled() || !cgroup_subsys_on_dfl(memory_cgrp_subsys))
		return false;

	memcg = folio_memcg(folio);
	if (!memcg)
		return false;

	for (; memcg != root_mem_cgroup; memcg = parent_mem_cgroup(memcg)) {
		unsigned long usage = page_counter_read(&memcg->swap);

		if (usage * 2 >= READ_ONCE(memcg->swap.high) ||
		    usage * 2 >= READ_ONCE(memcg->swap.max))
			return true;
	}

	return false;
}

struct mem_cgroup_swap_full__HEXSHA { void _b25806dcd3d5; };
struct mem_cgroup_swap_full__ATTRS { };
