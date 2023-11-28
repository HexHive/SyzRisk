static void kfd_set_iolink_no_atomics(struct kfd_topology_device *dev,
					struct kfd_topology_device *target_gpu_dev,
					struct kfd_iolink_properties *link)
{
	/* xgmi always supports atomics between links. */
	if (link->iolink_type == CRAT_IOLINK_TYPE_XGMI)
		return;

	/* check pcie support to set cpu(dev) flags for target_gpu_dev link. */
	if (target_gpu_dev) {
		uint32_t cap;

		pcie_capability_read_dword(target_gpu_dev->gpu->adev->pdev,
				PCI_EXP_DEVCAP2, &cap);

		if (!(cap & (PCI_EXP_DEVCAP2_ATOMIC_COMP32 |
			     PCI_EXP_DEVCAP2_ATOMIC_COMP64)))
			link->flags |= CRAT_IOLINK_FLAGS_NO_ATOMICS_32_BIT |
				CRAT_IOLINK_FLAGS_NO_ATOMICS_64_BIT;
	/* set gpu (dev) flags. */
	} else {
		if (!dev->gpu->pci_atomic_requested ||
				dev->gpu->adev->asic_type == CHIP_HAWAII)
			link->flags |= CRAT_IOLINK_FLAGS_NO_ATOMICS_32_BIT |
				CRAT_IOLINK_FLAGS_NO_ATOMICS_64_BIT;
	}
}

struct kfd_set_iolink_no_atomics__HEXSHA { void _d69a3b762dc4; };
struct kfd_set_iolink_no_atomics__ATTRS { };
