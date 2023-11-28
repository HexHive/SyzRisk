static int evict_folios(struct lruvec *lruvec, struct scan_control *sc, int swappiness,
			bool *need_swapping)
{
	int type;
	int scanned;
	int reclaimed;
	LIST_HEAD(list);
	struct folio *folio;
	enum vm_event_item item;
	struct reclaim_stat stat;
	struct lru_gen_mm_walk *walk;
	struct mem_cgroup *memcg = lruvec_memcg(lruvec);
	struct pglist_data *pgdat = lruvec_pgdat(lruvec);

	spin_lock_irq(&lruvec->lru_lock);

	scanned = isolate_folios(lruvec, sc, swappiness, &type, &list);

	scanned += try_to_inc_min_seq(lruvec, swappiness);

	if (get_nr_gens(lruvec, !swappiness) == MIN_NR_GENS)
		scanned = 0;

	spin_unlock_irq(&lruvec->lru_lock);

	if (list_empty(&list))
		return scanned;

	reclaimed = shrink_page_list(&list, pgdat, sc, &stat, false);

	list_for_each_entry(folio, &list, lru) {
		/* restore LRU_REFS_FLAGS cleared by isolate_folio() */
		if (folio_test_workingset(folio))
			folio_set_referenced(folio);

		/* don't add rejected pages to the oldest generation */
		if (folio_test_reclaim(folio) &&
		    (folio_test_dirty(folio) || folio_test_writeback(folio)))
			folio_clear_active(folio);
		else
			folio_set_active(folio);
	}

	spin_lock_irq(&lruvec->lru_lock);

	move_pages_to_lru(lruvec, &list);

	walk = current->reclaim_state->mm_walk;
	if (walk && walk->batched)
		reset_batch_size(lruvec, walk);

	item = current_is_kswapd() ? PGSTEAL_KSWAPD : PGSTEAL_DIRECT;
	if (!cgroup_reclaim(sc))
		__count_vm_events(item, reclaimed);
	__count_memcg_events(memcg, item, reclaimed);
	__count_vm_events(PGSTEAL_ANON + type, reclaimed);

	spin_unlock_irq(&lruvec->lru_lock);

	mem_cgroup_uncharge_list(&list);
	free_unref_page_list(&list);

	sc->nr_reclaimed += reclaimed;

	if (need_swapping && type == LRU_GEN_ANON)
		*need_swapping = true;

	return scanned;
}

struct evict_folios__HEXSHA { void _49fd9b6df54e; };
struct evict_folios__ATTRS { };
