struct kfd_dev *kfd_device_by_pci_dev(const struct pci_dev *pdev)
{
	struct kfd_topology_device *top_dev;
	struct kfd_dev *device = NULL;

	down_read(&topology_lock);

	list_for_each_entry(top_dev, &topology_device_list, list)
		if (top_dev->gpu && top_dev->gpu->adev->pdev == pdev) {
			device = top_dev->gpu;
			break;
		}

	up_read(&topology_lock);

	return device;
}

struct kfd_device_by_pci_dev__HEXSHA { void _d69a3b762dc4; };
struct kfd_device_by_pci_dev__ATTRS { };
