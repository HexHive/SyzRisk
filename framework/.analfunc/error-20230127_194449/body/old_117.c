int kfd_iommu_resume(struct kfd_dev *kfd)
{
	unsigned int pasid_limit;
	int err;

	if (!kfd->use_iommu_v2)
		return 0;

	pasid_limit = kfd_get_pasid_limit();

	err = amd_iommu_init_device(kfd->pdev, pasid_limit);
	if (err)
		return -ENXIO;

	amd_iommu_set_invalidate_ctx_cb(kfd->pdev,
					iommu_pasid_shutdown_callback);
	amd_iommu_set_invalid_ppr_cb(kfd->pdev,
				     iommu_invalid_ppr_cb);

	err = kfd_bind_processes_to_device(kfd);
	if (err) {
		amd_iommu_set_invalidate_ctx_cb(kfd->pdev, NULL);
		amd_iommu_set_invalid_ppr_cb(kfd->pdev, NULL);
		amd_iommu_free_device(kfd->pdev);
		return err;
	}

	return 0;
}

struct kfd_iommu_resume__HEXSHA { void _d69a3b762dc4; };
struct kfd_iommu_resume__ATTRS { };
