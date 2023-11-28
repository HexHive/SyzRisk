static int
nv50_dmac_wait(struct nvif_push *push, u32 size)
{
	struct nv50_dmac *dmac = container_of(push, typeof(*dmac), _push);
	int free;

	if (WARN_ON(size > dmac->max))
		return -EINVAL;

	dmac->cur = push->cur - (u32 *)dmac->_push.mem.object.map.ptr;
	if (dmac->cur + size >= dmac->max) {
		int ret = nv50_dmac_wind(dmac);
		if (ret)
			return ret;

		push->cur = dmac->_push.mem.object.map.ptr;
		push->cur = push->cur + dmac->cur;
		nv50_dmac_kick(push);
	}

	if (nvif_msec(dmac->base.device, 2000,
		if ((free = nv50_dmac_free(dmac)) >= size)
			break;
	) < 0) {
		WARN_ON(1);
		return -ETIMEDOUT;
	}

	push->bgn = dmac->_push.mem.object.map.ptr;
	push->bgn = push->bgn + dmac->cur;
	push->cur = push->bgn;
	push->end = push->cur + free;
	return 0;
}

struct nv50_dmac_wait__HEXSHA { void _9cf06d6ef7fd; };
struct nv50_dmac_wait__ATTRS { };
