static unsigned int demote_folio_list(struct list_head *demote_folios,
				     struct pglist_data *pgdat)
{
	int target_nid = next_demotion_node(pgdat->node_id);
	unsigned int nr_succeeded;
	nodemask_t allowed_mask;

	struct migration_target_control mtc = {
		/*
		 * Allocate from 'node', or fail quickly and quietly.
		 * When this happens, 'page' will likely just be discarded
		 * instead of migrated.
		 */
		.gfp_mask = (GFP_HIGHUSER_MOVABLE & ~__GFP_RECLAIM) | __GFP_NOWARN |
			__GFP_NOMEMALLOC | GFP_NOWAIT,
		.nid = target_nid,
		.nmask = &allowed_mask
	};

	if (list_empty(demote_folios))
		return 0;

	if (target_nid == NUMA_NO_NODE)
		return 0;

	node_get_allowed_targets(pgdat, &allowed_mask);

	/* Demotion ignores all cpuset and mempolicy settings */
	migrate_pages(demote_folios, alloc_demote_page, NULL,
		      (unsigned long)&mtc, MIGRATE_ASYNC, MR_DEMOTION,
		      &nr_succeeded);

	if (current_is_kswapd())
		__count_vm_events(PGDEMOTE_KSWAPD, nr_succeeded);
	else
		__count_vm_events(PGDEMOTE_DIRECT, nr_succeeded);

	return nr_succeeded;
}

struct demote_folio_list__HEXSHA { void _49fd9b6df54e; };
struct demote_folio_list__ATTRS { };
