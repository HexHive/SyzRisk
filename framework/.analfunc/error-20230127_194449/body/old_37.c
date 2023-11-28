static unsigned long isolate_lru_pages(unsigned long nr_to_scan,
		struct lruvec *lruvec, struct list_head *dst,
		unsigned long *nr_scanned, struct scan_control *sc,
		enum lru_list lru)
{
	struct list_head *src = &lruvec->lists[lru];
	unsigned long nr_taken = 0;
	unsigned long nr_zone_taken[MAX_NR_ZONES] = { 0 };
	unsigned long nr_skipped[MAX_NR_ZONES] = { 0, };
	unsigned long skipped = 0;
	unsigned long scan, total_scan, nr_pages;
	LIST_HEAD(folios_skipped);

	total_scan = 0;
	scan = 0;
	while (scan < nr_to_scan && !list_empty(src)) {
		struct list_head *move_to = src;
		struct folio *folio;

		folio = lru_to_folio(src);
		prefetchw_prev_lru_folio(folio, src, flags);

		nr_pages = folio_nr_pages(folio);
		total_scan += nr_pages;

		if (folio_zonenum(folio) > sc->reclaim_idx) {
			nr_skipped[folio_zonenum(folio)] += nr_pages;
			move_to = &folios_skipped;
			goto move;
		}

		/*
		 * Do not count skipped folios because that makes the function
		 * return with no isolated folios if the LRU mostly contains
		 * ineligible folios.  This causes the VM to not reclaim any
		 * folios, triggering a premature OOM.
		 * Account all pages in a folio.
		 */
		scan += nr_pages;

		if (!folio_test_lru(folio))
			goto move;
		if (!sc->may_unmap && folio_mapped(folio))
			goto move;

		/*
		 * Be careful not to clear the lru flag until after we're
		 * sure the folio is not being freed elsewhere -- the
		 * folio release code relies on it.
		 */
		if (unlikely(!folio_try_get(folio)))
			goto move;

		if (!folio_test_clear_lru(folio)) {
			/* Another thread is already isolating this folio */
			folio_put(folio);
			goto move;
		}

		nr_taken += nr_pages;
		nr_zone_taken[folio_zonenum(folio)] += nr_pages;
		move_to = dst;
move:
		list_move(&folio->lru, move_to);
	}

	/*
	 * Splice any skipped folios to the start of the LRU list. Note that
	 * this disrupts the LRU order when reclaiming for lower zones but
	 * we cannot splice to the tail. If we did then the SWAP_CLUSTER_MAX
	 * scanning would soon rescan the same folios to skip and waste lots
	 * of cpu cycles.
	 */
	if (!list_empty(&folios_skipped)) {
		int zid;

		list_splice(&folios_skipped, src);
		for (zid = 0; zid < MAX_NR_ZONES; zid++) {
			if (!nr_skipped[zid])
				continue;

			__count_zid_vm_events(PGSCAN_SKIP, zid, nr_skipped[zid]);
			skipped += nr_skipped[zid];
		}
	}
	*nr_scanned = total_scan;
	trace_mm_vmscan_lru_isolate(sc->reclaim_idx, sc->order, nr_to_scan,
				    total_scan, skipped, nr_taken,
				    sc->may_unmap ? 0 : ISOLATE_UNMAPPED, lru);
	update_lru_sizes(lruvec, lru, nr_zone_taken);
	return nr_taken;
}

struct isolate_lru_pages__HEXSHA { void _49fd9b6df54e; };
struct isolate_lru_pages__ATTRS { };
