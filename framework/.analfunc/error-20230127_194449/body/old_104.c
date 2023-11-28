void kfd_iommu_unbind_process(struct kfd_process *p)
{
	int i;

	for (i = 0; i < p->n_pdds; i++)
		if (p->pdds[i]->bound == PDD_BOUND)
			amd_iommu_unbind_pasid(p->pdds[i]->dev->pdev, p->pasid);
}

struct kfd_iommu_unbind_process__HEXSHA { void _d69a3b762dc4; };
struct kfd_iommu_unbind_process__ATTRS { };
