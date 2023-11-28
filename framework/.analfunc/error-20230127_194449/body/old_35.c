static inline int is_page_cache_freeable(struct folio *folio)
{
	/*
	 * A freeable page cache page is referenced only by the caller
	 * that isolated the page, the page cache and optional buffer
	 * heads at page->private.
	 */
	return folio_ref_count(folio) - folio_test_private(folio) ==
		1 + folio_nr_pages(folio);
}

struct is_page_cache_freeable__HEXSHA { void _49fd9b6df54e; };
struct is_page_cache_freeable__ATTRS { };
