static unsigned int reclaim_folio_list(struct list_head *page_list,
				      struct pglist_data *pgdat)
{
	struct reclaim_stat dummy_stat;
	unsigned int nr_reclaimed;
	struct folio *folio;
	struct scan_control sc = {
		.gfp_mask = GFP_KERNEL,
		.may_writepage = 1,
		.may_unmap = 1,
		.may_swap = 1,
		.no_demotion = 1,
	};

	nr_reclaimed = shrink_page_list(page_list, pgdat, &sc, &dummy_stat, false);
	while (!list_empty(page_list)) {
		folio = lru_to_folio(page_list);
		list_del(&folio->lru);
		folio_putback_lru(folio);
	}

	return nr_reclaimed;
}

struct reclaim_folio_list__HEXSHA { void _49fd9b6df54e; };
struct reclaim_folio_list__ATTRS { };
