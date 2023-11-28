static bool isolate_folio(struct lruvec *lruvec, struct folio *folio, struct scan_control *sc)
{
	bool success;

	/* unmapping inhibited */
	if (!sc->may_unmap && folio_mapped(folio))
		return false;

	/* swapping inhibited */
	if (!(sc->may_writepage && (sc->gfp_mask & __GFP_IO)) &&
	    (folio_test_dirty(folio) ||
	     (folio_test_anon(folio) && !folio_test_swapcache(folio))))
		return false;

	/* raced with release_pages() */
	if (!folio_try_get(folio))
		return false;

	/* raced with another isolation */
	if (!folio_test_clear_lru(folio)) {
		folio_put(folio);
		return false;
	}

	/* see the comment on MAX_NR_TIERS */
	if (!folio_test_referenced(folio))
		set_mask_bits(&folio->flags, LRU_REFS_MASK | LRU_REFS_FLAGS, 0);

	/* for shrink_folio_list() */
	folio_clear_reclaim(folio);
	folio_clear_referenced(folio);

	success = lru_gen_del_folio(lruvec, folio, true);
	VM_WARN_ON_ONCE_FOLIO(!success, folio);

	return true;
}

struct isolate_folio__HEXSHA { void _49fd9b6df54e; };
struct isolate_folio__ATTRS { };
