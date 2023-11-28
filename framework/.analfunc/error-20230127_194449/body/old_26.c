static enum page_references folio_check_references(struct folio *folio,
						  struct scan_control *sc)
{
	int referenced_ptes, referenced_folio;
	unsigned long vm_flags;

	referenced_ptes = folio_referenced(folio, 1, sc->target_mem_cgroup,
					   &vm_flags);
	referenced_folio = folio_test_clear_referenced(folio);

	/*
	 * The supposedly reclaimable folio was found to be in a VM_LOCKED vma.
	 * Let the folio, now marked Mlocked, be moved to the unevictable list.
	 */
	if (vm_flags & VM_LOCKED)
		return PAGEREF_ACTIVATE;

	/* rmap lock contention: rotate */
	if (referenced_ptes == -1)
		return PAGEREF_KEEP;

	if (referenced_ptes) {
		/*
		 * All mapped folios start out with page table
		 * references from the instantiating fault, so we need
		 * to look twice if a mapped file/anon folio is used more
		 * than once.
		 *
		 * Mark it and spare it for another trip around the
		 * inactive list.  Another page table reference will
		 * lead to its activation.
		 *
		 * Note: the mark is set for activated folios as well
		 * so that recently deactivated but used folios are
		 * quickly recovered.
		 */
		folio_set_referenced(folio);

		if (referenced_folio || referenced_ptes > 1)
			return PAGEREF_ACTIVATE;

		/*
		 * Activate file-backed executable folios after first usage.
		 */
		if ((vm_flags & VM_EXEC) && folio_is_file_lru(folio))
			return PAGEREF_ACTIVATE;

		return PAGEREF_KEEP;
	}

	/* Reclaim if clean, defer dirty folios to writeback */
	if (referenced_folio && folio_is_file_lru(folio))
		return PAGEREF_RECLAIM_CLEAN;

	return PAGEREF_RECLAIM;
}

struct folio_check_references__HEXSHA { void _49fd9b6df54e; };
struct folio_check_references__ATTRS { };
