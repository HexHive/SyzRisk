int kgd2kfd_resume_iommu(struct kfd_dev *kfd)
{
	int err = 0;

	err = kfd_iommu_resume(kfd);
	if (err)
		dev_err(kfd_device,
			"Failed to resume IOMMU for device %x:%x\n",
			kfd->adev->pdev->vendor, kfd->adev->pdev->device);
	return err;
}

struct kgd2kfd_resume_iommu__HEXSHA { void _d69a3b762dc4; };
struct kgd2kfd_resume_iommu__ATTRS { };
