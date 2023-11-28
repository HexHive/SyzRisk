void __mem_cgroup_uncharge_swap(swp_entry_t entry, unsigned int nr_pages)
{
	struct mem_cgroup *memcg;
	unsigned short id;

	if (mem_cgroup_disabled())
		return;

	id = swap_cgroup_record(entry, 0, nr_pages);
	rcu_read_lock();
	memcg = mem_cgroup_from_id(id);
	if (memcg) {
		if (memcg_swap_enabled() && !mem_cgroup_is_root(memcg)) {
			if (cgroup_subsys_on_dfl(memory_cgrp_subsys))
				page_counter_uncharge(&memcg->swap, nr_pages);
			else
				page_counter_uncharge(&memcg->memsw, nr_pages);
		}
		mod_memcg_state(memcg, MEMCG_SWAP, -nr_pages);
		mem_cgroup_id_put_many(memcg, nr_pages);
	}
	rcu_read_unlock();
}

struct __mem_cgroup_uncharge_swap__HEXSHA { void _b25806dcd3d5; };
struct __mem_cgroup_uncharge_swap__ATTRS { };
