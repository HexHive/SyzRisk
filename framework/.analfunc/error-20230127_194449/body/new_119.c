static int cxl_rr_alloc_decoder(struct cxl_port *port, struct cxl_region *cxlr,
				struct cxl_endpoint_decoder *cxled,
				struct cxl_region_ref *cxl_rr)
{
	struct cxl_decoder *cxld;

	if (port == cxled_to_port(cxled))
		cxld = &cxled->cxld;
	else
		cxld = cxl_region_find_decoder(port, cxlr);
	if (!cxld) {
		dev_dbg(&cxlr->dev, "%s: no decoder available\n",
			dev_name(&port->dev));
		return -EBUSY;
	}

	if (cxld->region) {
		dev_dbg(&cxlr->dev, "%s: %s already attached to %s\n",
			dev_name(&port->dev), dev_name(&cxld->dev),
			dev_name(&cxld->region->dev));
		return -EBUSY;
	}

	cxl_rr->decoder = cxld;
	return 0;
}

struct cxl_rr_alloc_decoder__HEXSHA { void _71ee71d7adcb; };
struct cxl_rr_alloc_decoder__ATTRS { };
