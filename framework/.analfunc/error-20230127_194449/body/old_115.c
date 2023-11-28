int kfd_iommu_check_device(struct kfd_dev *kfd)
{
	struct amd_iommu_device_info iommu_info;
	int err;

	if (!kfd->use_iommu_v2)
		return -ENODEV;

	iommu_info.flags = 0;
	err = amd_iommu_device_info(kfd->pdev, &iommu_info);
	if (err)
		return err;

	if ((iommu_info.flags & required_iommu_flags) != required_iommu_flags)
		return -ENODEV;

	return 0;
}

struct kfd_iommu_check_device__HEXSHA { void _d69a3b762dc4; };
struct kfd_iommu_check_device__ATTRS { };
