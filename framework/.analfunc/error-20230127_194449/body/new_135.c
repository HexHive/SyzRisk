int kfd_iommu_device_init(struct kfd_dev *kfd)
{
	struct amd_iommu_device_info iommu_info;
	unsigned int pasid_limit;
	int err;

	if (!kfd->use_iommu_v2)
		return 0;

	iommu_info.flags = 0;
	err = amd_iommu_device_info(kfd->adev->pdev, &iommu_info);
	if (err < 0) {
		dev_err(kfd_device,
			"error getting iommu info. is the iommu enabled?\n");
		return -ENODEV;
	}

	if ((iommu_info.flags & required_iommu_flags) != required_iommu_flags) {
		dev_err(kfd_device,
			"error required iommu flags ats %i, pri %i, pasid %i\n",
		       (iommu_info.flags & AMD_IOMMU_DEVICE_FLAG_ATS_SUP) != 0,
		       (iommu_info.flags & AMD_IOMMU_DEVICE_FLAG_PRI_SUP) != 0,
		       (iommu_info.flags & AMD_IOMMU_DEVICE_FLAG_PASID_SUP)
									!= 0);
		return -ENODEV;
	}

	pasid_limit = min_t(unsigned int,
			(unsigned int)(1 << kfd->device_info.max_pasid_bits),
			iommu_info.max_pasids);

	if (!kfd_set_pasid_limit(pasid_limit)) {
		dev_err(kfd_device, "error setting pasid limit\n");
		return -EBUSY;
	}

	return 0;
}

struct kfd_iommu_device_init__HEXSHA { void _d69a3b762dc4; };
struct kfd_iommu_device_init__ATTRS { };
