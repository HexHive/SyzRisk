static void
nv50_dmac_kick(struct nvif_push *push)
{
	struct nv50_dmac *dmac = container_of(push, typeof(*dmac), _push);

	dmac->cur = push->cur - (u32 __iomem *)dmac->_push.mem.object.map.ptr;
	if (dmac->put != dmac->cur) {
		/* Push buffer fetches are not coherent with BAR1, we need to ensure
		 * writes have been flushed right through to VRAM before writing PUT.
		 */
		if (dmac->push->mem.type & NVIF_MEM_VRAM) {
			struct nvif_device *device = dmac->base.device;
			nvif_wr32(&device->object, 0x070000, 0x00000001);
			nvif_msec(device, 2000,
				if (!(nvif_rd32(&device->object, 0x070000) & 0x00000002))
					break;
			);
		}

		NVIF_WV32(&dmac->base.user, NV507C, PUT, PTR, dmac->cur);
		dmac->put = dmac->cur;
	}

	push->bgn = push->cur;
}

struct nv50_dmac_kick__HEXSHA { void _9cf06d6ef7fd; };
struct nv50_dmac_kick__ATTRS { };
