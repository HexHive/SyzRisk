void svm_range_free_dma_mappings(struct svm_range *prange)
{
	struct kfd_process_device *pdd;
	dma_addr_t *dma_addr;
	struct device *dev;
	struct kfd_process *p;
	uint32_t gpuidx;

	p = container_of(prange->svms, struct kfd_process, svms);

	for (gpuidx = 0; gpuidx < MAX_GPU_INSTANCE; gpuidx++) {
		dma_addr = prange->dma_addr[gpuidx];
		if (!dma_addr)
			continue;

		pdd = kfd_process_device_from_gpuidx(p, gpuidx);
		if (!pdd) {
			pr_debug("failed to find device idx %d\n", gpuidx);
			continue;
		}
		dev = &pdd->dev->adev->pdev->dev;
		svm_range_dma_unmap(dev, dma_addr, 0, prange->npages);
		kvfree(dma_addr);
		prange->dma_addr[gpuidx] = NULL;
	}
}

struct svm_range_free_dma_mappings__HEXSHA { void _d69a3b762dc4; };
struct svm_range_free_dma_mappings__ATTRS { };
