static unsigned int shrink_folio_list(struct list_head *folio_list,
		struct pglist_data *pgdat, struct scan_control *sc,
		struct reclaim_stat *stat, bool ignore_references)
{
	LIST_HEAD(ret_folios);
	LIST_HEAD(free_folios);
	LIST_HEAD(demote_folios);
	unsigned int nr_reclaimed = 0;
	unsigned int pgactivate = 0;
	bool do_demote_pass;
	struct swap_iocb *plug = NULL;

	memset(stat, 0, sizeof(*stat));
	cond_resched();
	do_demote_pass = can_demote(pgdat->node_id, sc);

retry:
	while (!list_empty(folio_list)) {
		struct address_space *mapping;
		struct folio *folio;
		enum folio_references references = FOLIOREF_RECLAIM;
		bool dirty, writeback;
		unsigned int nr_pages;

		cond_resched();

		folio = lru_to_folio(folio_list);
		list_del(&folio->lru);

		if (!folio_trylock(folio))
			goto keep;

		VM_BUG_ON_FOLIO(folio_test_active(folio), folio);

		nr_pages = folio_nr_pages(folio);

		/* Account the number of base pages */
		sc->nr_scanned += nr_pages;

		if (unlikely(!folio_evictable(folio)))
			goto activate_locked;

		if (!sc->may_unmap && folio_mapped(folio))
			goto keep_locked;

		/* folio_update_gen() tried to promote this page? */
		if (lru_gen_enabled() && !ignore_references &&
		    folio_mapped(folio) && folio_test_referenced(folio))
			goto keep_locked;

		/*
		 * The number of dirty pages determines if a node is marked
		 * reclaim_congested. kswapd will stall and start writing
		 * folios if the tail of the LRU is all dirty unqueued folios.
		 */
		folio_check_dirty_writeback(folio, &dirty, &writeback);
		if (dirty || writeback)
			stat->nr_dirty += nr_pages;

		if (dirty && !writeback)
			stat->nr_unqueued_dirty += nr_pages;

		/*
		 * Treat this folio as congested if folios are cycling
		 * through the LRU so quickly that the folios marked
		 * for immediate reclaim are making it to the end of
		 * the LRU a second time.
		 */
		if (writeback && folio_test_reclaim(folio))
			stat->nr_congested += nr_pages;

		/*
		 * If a folio at the tail of the LRU is under writeback, there
		 * are three cases to consider.
		 *
		 * 1) If reclaim is encountering an excessive number
		 *    of folios under writeback and this folio has both
		 *    the writeback and reclaim flags set, then it
		 *    indicates that folios are being queued for I/O but
		 *    are being recycled through the LRU before the I/O
		 *    can complete. Waiting on the folio itself risks an
		 *    indefinite stall if it is impossible to writeback
		 *    the folio due to I/O error or disconnected storage
		 *    so instead note that the LRU is being scanned too
		 *    quickly and the caller can stall after the folio
		 *    list has been processed.
		 *
		 * 2) Global or new memcg reclaim encounters a folio that is
		 *    not marked for immediate reclaim, or the caller does not
		 *    have __GFP_FS (or __GFP_IO if it's simply going to swap,
		 *    not to fs). In this case mark the folio for immediate
		 *    reclaim and continue scanning.
		 *
		 *    Require may_enter_fs() because we would wait on fs, which
		 *    may not have submitted I/O yet. And the loop driver might
		 *    enter reclaim, and deadlock if it waits on a folio for
		 *    which it is needed to do the write (loop masks off
		 *    __GFP_IO|__GFP_FS for this reason); but more thought
		 *    would probably show more reasons.
		 *
		 * 3) Legacy memcg encounters a folio that already has the
		 *    reclaim flag set. memcg does not have any dirty folio
		 *    throttling so we could easily OOM just because too many
		 *    folios are in writeback and there is nothing else to
		 *    reclaim. Wait for the writeback to complete.
		 *
		 * In cases 1) and 2) we activate the folios to get them out of
		 * the way while we continue scanning for clean folios on the
		 * inactive list and refilling from the active list. The
		 * observation here is that waiting for disk writes is more
		 * expensive than potentially causing reloads down the line.
		 * Since they're marked for immediate reclaim, they won't put
		 * memory pressure on the cache working set any longer than it
		 * takes to write them to disk.
		 */
		if (folio_test_writeback(folio)) {
			/* Case 1 above */
			if (current_is_kswapd() &&
			    folio_test_reclaim(folio) &&
			    test_bit(PGDAT_WRITEBACK, &pgdat->flags)) {
				stat->nr_immediate += nr_pages;
				goto activate_locked;

			/* Case 2 above */
			} else if (writeback_throttling_sane(sc) ||
			    !folio_test_reclaim(folio) ||
			    !may_enter_fs(folio, sc->gfp_mask)) {
				/*
				 * This is slightly racy -
				 * folio_end_writeback() might have
				 * just cleared the reclaim flag, then
				 * setting the reclaim flag here ends up
				 * interpreted as the readahead flag - but
				 * that does not matter enough to care.
				 * What we do want is for this folio to
				 * have the reclaim flag set next time
				 * memcg reclaim reaches the tests above,
				 * so it will then wait for writeback to
				 * avoid OOM; and it's also appropriate
				 * in global reclaim.
				 */
				folio_set_reclaim(folio);
				stat->nr_writeback += nr_pages;
				goto activate_locked;

			/* Case 3 above */
			} else {
				folio_unlock(folio);
				folio_wait_writeback(folio);
				/* then go back and try same folio again */
				list_add_tail(&folio->lru, folio_list);
				continue;
			}
		}

		if (!ignore_references)
			references = folio_check_references(folio, sc);

		switch (references) {
		case FOLIOREF_ACTIVATE:
			goto activate_locked;
		case FOLIOREF_KEEP:
			stat->nr_ref_keep += nr_pages;
			goto keep_locked;
		case FOLIOREF_RECLAIM:
		case FOLIOREF_RECLAIM_CLEAN:
			; /* try to reclaim the folio below */
		}

		/*
		 * Before reclaiming the folio, try to relocate
		 * its contents to another node.
		 */
		if (do_demote_pass &&
		    (thp_migration_supported() || !folio_test_large(folio))) {
			list_add(&folio->lru, &demote_folios);
			folio_unlock(folio);
			continue;
		}

		/*
		 * Anonymous process memory has backing store?
		 * Try to allocate it some swap space here.
		 * Lazyfree folio could be freed directly
		 */
		if (folio_test_anon(folio) && folio_test_swapbacked(folio)) {
			if (!folio_test_swapcache(folio)) {
				if (!(sc->gfp_mask & __GFP_IO))
					goto keep_locked;
				if (folio_maybe_dma_pinned(folio))
					goto keep_locked;
				if (folio_test_large(folio)) {
					/* cannot split folio, skip it */
					if (!can_split_folio(folio, NULL))
						goto activate_locked;
					/*
					 * Split folios without a PMD map right
					 * away. Chances are some or all of the
					 * tail pages can be freed without IO.
					 */
					if (!folio_entire_mapcount(folio) &&
					    split_folio_to_list(folio,
								folio_list))
						goto activate_locked;
				}
				if (!add_to_swap(folio)) {
					if (!folio_test_large(folio))
						goto activate_locked_split;
					/* Fallback to swap normal pages */
					if (split_folio_to_list(folio,
								folio_list))
						goto activate_locked;

					count_vm_event(THP_SWPOUT_FALLBACK);

					if (!add_to_swap(folio))
						goto activate_locked_split;
				}
			}
		} else if (folio_test_swapbacked(folio) &&
			   folio_test_large(folio)) {
			/* Split shmem folio */
			if (split_folio_to_list(folio, folio_list))
				goto keep_locked;
		}

		/*
		 * If the folio was split above, the tail pages will make
		 * their own pass through this function and be accounted
		 * then.
		 */
		if ((nr_pages > 1) && !folio_test_large(folio)) {
			sc->nr_scanned -= (nr_pages - 1);
			nr_pages = 1;
		}

		/*
		 * The folio is mapped into the page tables of one or more
		 * processes. Try to unmap it here.
		 */
		if (folio_mapped(folio)) {
			enum ttu_flags flags = TTU_BATCH_FLUSH;
			bool was_swapbacked = folio_test_swapbacked(folio);

			if (folio_test_pmd_mappable(folio))
				flags |= TTU_SPLIT_HUGE_PMD;

			try_to_unmap(folio, flags);
			if (folio_mapped(folio)) {
				stat->nr_unmap_fail += nr_pages;
				if (!was_swapbacked &&
				    folio_test_swapbacked(folio))
					stat->nr_lazyfree_fail += nr_pages;
				goto activate_locked;
			}
		}

		mapping = folio_mapping(folio);
		if (folio_test_dirty(folio)) {
			/*
			 * Only kswapd can writeback filesystem folios
			 * to avoid risk of stack overflow. But avoid
			 * injecting inefficient single-folio I/O into
			 * flusher writeback as much as possible: only
			 * write folios when we've encountered many
			 * dirty folios, and when we've already scanned
			 * the rest of the LRU for clean folios and see
			 * the same dirty folios again (with the reclaim
			 * flag set).
			 */
			if (folio_is_file_lru(folio) &&
			    (!current_is_kswapd() ||
			     !folio_test_reclaim(folio) ||
			     !test_bit(PGDAT_DIRTY, &pgdat->flags))) {
				/*
				 * Immediately reclaim when written back.
				 * Similar in principle to deactivate_page()
				 * except we already have the folio isolated
				 * and know it's dirty
				 */
				node_stat_mod_folio(folio, NR_VMSCAN_IMMEDIATE,
						nr_pages);
				folio_set_reclaim(folio);

				goto activate_locked;
			}

			if (references == FOLIOREF_RECLAIM_CLEAN)
				goto keep_locked;
			if (!may_enter_fs(folio, sc->gfp_mask))
				goto keep_locked;
			if (!sc->may_writepage)
				goto keep_locked;

			/*
			 * Folio is dirty. Flush the TLB if a writable entry
			 * potentially exists to avoid CPU writes after I/O
			 * starts and then write it out here.
			 */
			try_to_unmap_flush_dirty();
			switch (pageout(folio, mapping, &plug)) {
			case PAGE_KEEP:
				goto keep_locked;
			case PAGE_ACTIVATE:
				goto activate_locked;
			case PAGE_SUCCESS:
				stat->nr_pageout += nr_pages;

				if (folio_test_writeback(folio))
					goto keep;
				if (folio_test_dirty(folio))
					goto keep;

				/*
				 * A synchronous write - probably a ramdisk.  Go
				 * ahead and try to reclaim the folio.
				 */
				if (!folio_trylock(folio))
					goto keep;
				if (folio_test_dirty(folio) ||
				    folio_test_writeback(folio))
					goto keep_locked;
				mapping = folio_mapping(folio);
				fallthrough;
			case PAGE_CLEAN:
				; /* try to free the folio below */
			}
		}

		/*
		 * If the folio has buffers, try to free the buffer
		 * mappings associated with this folio. If we succeed
		 * we try to free the folio as well.
		 *
		 * We do this even if the folio is dirty.
		 * filemap_release_folio() does not perform I/O, but it
		 * is possible for a folio to have the dirty flag set,
		 * but it is actually clean (all its buffers are clean).
		 * This happens if the buffers were written out directly,
		 * with submit_bh(). ext3 will do this, as well as
		 * the blockdev mapping.  filemap_release_folio() will
		 * discover that cleanness and will drop the buffers
		 * and mark the folio clean - it can be freed.
		 *
		 * Rarely, folios can have buffers and no ->mapping.
		 * These are the folios which were not successfully
		 * invalidated in truncate_cleanup_folio().  We try to
		 * drop those buffers here and if that worked, and the
		 * folio is no longer mapped into process address space
		 * (refcount == 1) it can be freed.  Otherwise, leave
		 * the folio on the LRU so it is swappable.
		 */
		if (folio_has_private(folio)) {
			if (!filemap_release_folio(folio, sc->gfp_mask))
				goto activate_locked;
			if (!mapping && folio_ref_count(folio) == 1) {
				folio_unlock(folio);
				if (folio_put_testzero(folio))
					goto free_it;
				else {
					/*
					 * rare race with speculative reference.
					 * the speculative reference will free
					 * this folio shortly, so we may
					 * increment nr_reclaimed here (and
					 * leave it off the LRU).
					 */
					nr_reclaimed += nr_pages;
					continue;
				}
			}
		}

		if (folio_test_anon(folio) && !folio_test_swapbacked(folio)) {
			/* follow __remove_mapping for reference */
			if (!folio_ref_freeze(folio, 1))
				goto keep_locked;
			/*
			 * The folio has only one reference left, which is
			 * from the isolation. After the caller puts the
			 * folio back on the lru and drops the reference, the
			 * folio will be freed anyway. It doesn't matter
			 * which lru it goes on. So we don't bother checking
			 * the dirty flag here.
			 */
			count_vm_events(PGLAZYFREED, nr_pages);
			count_memcg_folio_events(folio, PGLAZYFREED, nr_pages);
		} else if (!mapping || !__remove_mapping(mapping, folio, true,
							 sc->target_mem_cgroup))
			goto keep_locked;

		folio_unlock(folio);
free_it:
		/*
		 * Folio may get swapped out as a whole, need to account
		 * all pages in it.
		 */
		nr_reclaimed += nr_pages;

		/*
		 * Is there need to periodically free_folio_list? It would
		 * appear not as the counts should be low
		 */
		if (unlikely(folio_test_large(folio)))
			destroy_large_folio(folio);
		else
			list_add(&folio->lru, &free_folios);
		continue;

activate_locked_split:
		/*
		 * The tail pages that are failed to add into swap cache
		 * reach here.  Fixup nr_scanned and nr_pages.
		 */
		if (nr_pages > 1) {
			sc->nr_scanned -= (nr_pages - 1);
			nr_pages = 1;
		}
activate_locked:
		/* Not a candidate for swapping, so reclaim swap space. */
		if (folio_test_swapcache(folio) &&
		    (mem_cgroup_swap_full(&folio->page) ||
		     folio_test_mlocked(folio)))
			try_to_free_swap(&folio->page);
		VM_BUG_ON_FOLIO(folio_test_active(folio), folio);
		if (!folio_test_mlocked(folio)) {
			int type = folio_is_file_lru(folio);
			folio_set_active(folio);
			stat->nr_activate[type] += nr_pages;
			count_memcg_folio_events(folio, PGACTIVATE, nr_pages);
		}
keep_locked:
		folio_unlock(folio);
keep:
		list_add(&folio->lru, &ret_folios);
		VM_BUG_ON_FOLIO(folio_test_lru(folio) ||
				folio_test_unevictable(folio), folio);
	}
	/* 'folio_list' is always empty here */

	/* Migrate folios selected for demotion */
	nr_reclaimed += demote_folio_list(&demote_folios, pgdat);
	/* Folios that could not be demoted are still in @demote_folios */
	if (!list_empty(&demote_folios)) {
		/* Folios which weren't demoted go back on @folio_list for retry: */
		list_splice_init(&demote_folios, folio_list);
		do_demote_pass = false;
		goto retry;
	}

	pgactivate = stat->nr_activate[0] + stat->nr_activate[1];

	mem_cgroup_uncharge_list(&free_folios);
	try_to_unmap_flush();
	free_unref_page_list(&free_folios);

	list_splice(&ret_folios, folio_list);
	count_vm_events(PGACTIVATE, pgactivate);

	if (plug)
		swap_write_unplug(plug);
	return nr_reclaimed;
}

struct shrink_folio_list__HEXSHA { void _49fd9b6df54e; };
struct shrink_folio_list__ATTRS { };
