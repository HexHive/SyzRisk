void mem_cgroup_swapout(struct folio *folio, swp_entry_t entry)
{
	struct mem_cgroup *memcg, *swap_memcg;
	unsigned int nr_entries;
	unsigned short oldid;

	VM_BUG_ON_FOLIO(folio_test_lru(folio), folio);
	VM_BUG_ON_FOLIO(folio_ref_count(folio), folio);

	if (mem_cgroup_disabled())
		return;

	if (cgroup_subsys_on_dfl(memory_cgrp_subsys))
		return;

	memcg = folio_memcg(folio);

	VM_WARN_ON_ONCE_FOLIO(!memcg, folio);
	if (!memcg)
		return;

	/*
	 * In case the memcg owning these pages has been offlined and doesn't
	 * have an ID allocated to it anymore, charge the closest online
	 * ancestor for the swap instead and transfer the memory+swap charge.
	 */
	swap_memcg = mem_cgroup_id_get_online(memcg);
	nr_entries = folio_nr_pages(folio);
	/* Get references for the tail pages, too */
	if (nr_entries > 1)
		mem_cgroup_id_get_many(swap_memcg, nr_entries - 1);
	oldid = swap_cgroup_record(entry, mem_cgroup_id(swap_memcg),
				   nr_entries);
	VM_BUG_ON_FOLIO(oldid, folio);
	mod_memcg_state(swap_memcg, MEMCG_SWAP, nr_entries);

	folio->memcg_data = 0;

	if (!mem_cgroup_is_root(memcg))
		page_counter_uncharge(&memcg->memory, nr_entries);

	if (memcg != swap_memcg) {
		if (!mem_cgroup_is_root(swap_memcg))
			page_counter_charge(&swap_memcg->memsw, nr_entries);
		page_counter_uncharge(&memcg->memsw, nr_entries);
	}

	/*
	 * Interrupts should be disabled here because the caller holds the
	 * i_pages lock which is taken with interrupts-off. It is
	 * important here to have the interrupts disabled because it is the
	 * only synchronisation we have for updating the per-CPU variables.
	 */
	memcg_stats_lock();
	mem_cgroup_charge_statistics(memcg, -nr_entries);
	memcg_stats_unlock();
	memcg_check_events(memcg, folio_nid(folio));

	css_put(&memcg->css);
}

struct mem_cgroup_swapout__HEXSHA { void _b25806dcd3d5; };
struct mem_cgroup_swapout__ATTRS { };
