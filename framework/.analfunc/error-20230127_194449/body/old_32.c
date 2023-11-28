static unsigned int move_folios_to_lru(struct lruvec *lruvec,
				      struct list_head *list)
{
	int nr_pages, nr_moved = 0;
	LIST_HEAD(folios_to_free);

	while (!list_empty(list)) {
		struct folio *folio = lru_to_folio(list);

		VM_BUG_ON_FOLIO(folio_test_lru(folio), folio);
		list_del(&folio->lru);
		if (unlikely(!folio_evictable(folio))) {
			spin_unlock_irq(&lruvec->lru_lock);
			folio_putback_lru(folio);
			spin_lock_irq(&lruvec->lru_lock);
			continue;
		}

		/*
		 * The folio_set_lru needs to be kept here for list integrity.
		 * Otherwise:
		 *   #0 move_pages_to_lru             #1 release_pages
		 *   if (!folio_put_testzero())
		 *				      if (folio_put_testzero())
		 *				        !lru //skip lru_lock
		 *     folio_set_lru()
		 *     list_add(&folio->lru,)
		 *                                        list_add(&folio->lru,)
		 */
		folio_set_lru(folio);

		if (unlikely(folio_put_testzero(folio))) {
			__folio_clear_lru_flags(folio);

			if (unlikely(folio_test_large(folio))) {
				spin_unlock_irq(&lruvec->lru_lock);
				destroy_large_folio(folio);
				spin_lock_irq(&lruvec->lru_lock);
			} else
				list_add(&folio->lru, &folios_to_free);

			continue;
		}

		/*
		 * All pages were isolated from the same lruvec (and isolation
		 * inhibits memcg migration).
		 */
		VM_BUG_ON_FOLIO(!folio_matches_lruvec(folio, lruvec), folio);
		lruvec_add_folio(lruvec, folio);
		nr_pages = folio_nr_pages(folio);
		nr_moved += nr_pages;
		if (folio_test_active(folio))
			workingset_age_nonresident(lruvec, nr_pages);
	}

	/*
	 * To save our caller's stack, now use input list for pages to free.
	 */
	list_splice(&folios_to_free, list);

	return nr_moved;
}

struct move_folios_to_lru__HEXSHA { void _49fd9b6df54e; };
struct move_folios_to_lru__ATTRS { };
