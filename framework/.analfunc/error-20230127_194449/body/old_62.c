int __mem_cgroup_try_charge_swap(struct folio *folio, swp_entry_t entry)
{
	unsigned int nr_pages = folio_nr_pages(folio);
	struct page_counter *counter;
	struct mem_cgroup *memcg;
	unsigned short oldid;

	if (!cgroup_subsys_on_dfl(memory_cgrp_subsys))
		return 0;

	memcg = folio_memcg(folio);

	VM_WARN_ON_ONCE_FOLIO(!memcg, folio);
	if (!memcg)
		return 0;

	if (!entry.val) {
		memcg_memory_event(memcg, MEMCG_SWAP_FAIL);
		return 0;
	}

	memcg = mem_cgroup_id_get_online(memcg);

	if (memcg_swap_enabled() && !mem_cgroup_is_root(memcg) &&
	    !page_counter_try_charge(&memcg->swap, nr_pages, &counter)) {
		memcg_memory_event(memcg, MEMCG_SWAP_MAX);
		memcg_memory_event(memcg, MEMCG_SWAP_FAIL);
		mem_cgroup_id_put(memcg);
		return -ENOMEM;
	}

	/* Get references for the tail pages, too */
	if (nr_pages > 1)
		mem_cgroup_id_get_many(memcg, nr_pages - 1);
	oldid = swap_cgroup_record(entry, mem_cgroup_id(memcg), nr_pages);
	VM_BUG_ON_FOLIO(oldid, folio);
	mod_memcg_state(memcg, MEMCG_SWAP, nr_pages);

	return 0;
}

struct __mem_cgroup_try_charge_swap__HEXSHA { void _b25806dcd3d5; };
struct __mem_cgroup_try_charge_swap__ATTRS { };
