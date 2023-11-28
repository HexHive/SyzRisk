static uint32_t kfd_generate_gpu_id(struct kfd_dev *gpu)
{
	uint32_t hashout;
	uint32_t buf[7];
	uint64_t local_mem_size;
	int i;

	if (!gpu)
		return 0;

	local_mem_size = gpu->local_mem_info.local_mem_size_private +
			gpu->local_mem_info.local_mem_size_public;

	buf[0] = gpu->pdev->devfn;
	buf[1] = gpu->pdev->subsystem_vendor |
		(gpu->pdev->subsystem_device << 16);
	buf[2] = pci_domain_nr(gpu->pdev->bus);
	buf[3] = gpu->pdev->device;
	buf[4] = gpu->pdev->bus->number;
	buf[5] = lower_32_bits(local_mem_size);
	buf[6] = upper_32_bits(local_mem_size);

	for (i = 0, hashout = 0; i < 7; i++)
		hashout ^= hash_32(buf[i], KFD_GPU_ID_HASH_WIDTH);

	return hashout;
}

struct kfd_generate_gpu_id__HEXSHA { void _d69a3b762dc4; };
struct kfd_generate_gpu_id__ATTRS { };
