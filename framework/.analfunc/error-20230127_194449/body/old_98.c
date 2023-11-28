static int cxl_port_attach_region(struct cxl_port *port,
				  struct cxl_region *cxlr,
				  struct cxl_endpoint_decoder *cxled, int pos)
{
	struct cxl_memdev *cxlmd = cxled_to_memdev(cxled);
	struct cxl_ep *ep = cxl_ep_load(port, cxlmd);
	struct cxl_region_ref *cxl_rr;
	bool nr_targets_inc = false;
	struct cxl_decoder *cxld;
	unsigned long index;
	int rc = -EBUSY;

	lockdep_assert_held_write(&cxl_region_rwsem);

	cxl_rr = cxl_rr_load(port, cxlr);
	if (cxl_rr) {
		struct cxl_ep *ep_iter;
		int found = 0;

		/*
		 * Walk the existing endpoints that have been attached to
		 * @cxlr at @port and see if they share the same 'next' port
		 * in the downstream direction. I.e. endpoints that share common
		 * upstream switch.
		 */
		xa_for_each(&cxl_rr->endpoints, index, ep_iter) {
			if (ep_iter == ep)
				continue;
			if (ep_iter->next == ep->next) {
				found++;
				break;
			}
		}

		/*
		 * New target port, or @port is an endpoint port that always
		 * accounts its own local decode as a target.
		 */
		if (!found || !ep->next) {
			cxl_rr->nr_targets++;
			nr_targets_inc = true;
		}

		/*
		 * The decoder for @cxlr was allocated when the region was first
		 * attached to @port.
		 */
		cxld = cxl_rr->decoder;
	} else {
		cxl_rr = alloc_region_ref(port, cxlr);
		if (IS_ERR(cxl_rr)) {
			dev_dbg(&cxlr->dev,
				"%s: failed to allocate region reference\n",
				dev_name(&port->dev));
			return PTR_ERR(cxl_rr);
		}
		nr_targets_inc = true;

		if (port == cxled_to_port(cxled))
			cxld = &cxled->cxld;
		else
			cxld = cxl_region_find_decoder(port, cxlr);
		if (!cxld) {
			dev_dbg(&cxlr->dev, "%s: no decoder available\n",
				dev_name(&port->dev));
			goto out_erase;
		}

		if (cxld->region) {
			dev_dbg(&cxlr->dev, "%s: %s already attached to %s\n",
				dev_name(&port->dev), dev_name(&cxld->dev),
				dev_name(&cxld->region->dev));
			rc = -EBUSY;
			goto out_erase;
		}

		cxl_rr->decoder = cxld;
	}

	rc = cxl_rr_ep_add(cxl_rr, cxled);
	if (rc) {
		dev_dbg(&cxlr->dev,
			"%s: failed to track endpoint %s:%s reference\n",
			dev_name(&port->dev), dev_name(&cxlmd->dev),
			dev_name(&cxld->dev));
		goto out_erase;
	}

	dev_dbg(&cxlr->dev,
		"%s:%s %s add: %s:%s @ %d next: %s nr_eps: %d nr_targets: %d\n",
		dev_name(port->uport), dev_name(&port->dev),
		dev_name(&cxld->dev), dev_name(&cxlmd->dev),
		dev_name(&cxled->cxld.dev), pos,
		ep ? ep->next ? dev_name(ep->next->uport) :
				      dev_name(&cxlmd->dev) :
			   "none",
		cxl_rr->nr_eps, cxl_rr->nr_targets);

	return 0;
out_erase:
	if (nr_targets_inc)
		cxl_rr->nr_targets--;
	if (cxl_rr->nr_eps == 0)
		free_region_ref(cxl_rr);
	return rc;
}

struct cxl_port_attach_region__HEXSHA { void _71ee71d7adcb; };
struct cxl_port_attach_region__ATTRS { };
