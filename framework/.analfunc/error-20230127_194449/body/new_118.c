static void free_region_ref(struct cxl_region_ref *cxl_rr)
{
	struct cxl_port *port = cxl_rr->port;
	struct cxl_region *cxlr = cxl_rr->region;

	cxl_rr_free_decoder(cxl_rr);
	xa_erase(&port->regions, (unsigned long)cxlr);
	xa_destroy(&cxl_rr->endpoints);
	kfree(cxl_rr);
}

struct free_region_ref__HEXSHA { void _71ee71d7adcb; };
struct free_region_ref__ATTRS { };
