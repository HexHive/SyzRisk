static void cxl_rr_free_decoder(struct cxl_region_ref *cxl_rr)
{
	struct cxl_region *cxlr = cxl_rr->region;
	struct cxl_decoder *cxld = cxl_rr->decoder;

	if (!cxld)
		return;

	dev_WARN_ONCE(&cxlr->dev, cxld->region != cxlr, "region mismatch\n");
	if (cxld->region == cxlr) {
		cxld->region = NULL;
		put_device(&cxlr->dev);
	}
}

struct cxl_rr_free_decoder__HEXSHA { void _71ee71d7adcb; };
struct cxl_rr_free_decoder__ATTRS { };
