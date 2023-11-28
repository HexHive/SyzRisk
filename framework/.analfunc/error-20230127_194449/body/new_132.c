void kfd_iommu_suspend(struct kfd_dev *kfd)
{
	if (!kfd->use_iommu_v2)
		return;

	kfd_unbind_processes_from_device(kfd);

	amd_iommu_set_invalidate_ctx_cb(kfd->adev->pdev, NULL);
	amd_iommu_set_invalid_ppr_cb(kfd->adev->pdev, NULL);
	amd_iommu_free_device(kfd->adev->pdev);
}

struct kfd_iommu_suspend__HEXSHA { void _d69a3b762dc4; };
struct kfd_iommu_suspend__ATTRS { };
