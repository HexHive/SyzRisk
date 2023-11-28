int kfd_iommu_bind_process_to_device(struct kfd_process_device *pdd)
{
	struct kfd_dev *dev = pdd->dev;
	struct kfd_process *p = pdd->process;
	int err;

	if (!dev->use_iommu_v2 || pdd->bound == PDD_BOUND)
		return 0;

	if (unlikely(pdd->bound == PDD_BOUND_SUSPENDED)) {
		pr_err("Binding PDD_BOUND_SUSPENDED pdd is unexpected!\n");
		return -EINVAL;
	}

	err = amd_iommu_bind_pasid(dev->adev->pdev, p->pasid, p->lead_thread);
	if (!err)
		pdd->bound = PDD_BOUND;

	return err;
}

struct kfd_iommu_bind_process_to_device__HEXSHA { void _d69a3b762dc4; };
struct kfd_iommu_bind_process_to_device__ATTRS { };
