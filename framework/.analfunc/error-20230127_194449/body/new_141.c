bool kgd2kfd_device_init(struct kfd_dev *kfd,
			 const struct kgd2kfd_shared_resources *gpu_resources)
{
	unsigned int size, map_process_packet_size;

	kfd->mec_fw_version = amdgpu_amdkfd_get_fw_version(kfd->adev,
			KGD_ENGINE_MEC1);
	kfd->mec2_fw_version = amdgpu_amdkfd_get_fw_version(kfd->adev,
			KGD_ENGINE_MEC2);
	kfd->sdma_fw_version = amdgpu_amdkfd_get_fw_version(kfd->adev,
			KGD_ENGINE_SDMA1);
	kfd->shared_resources = *gpu_resources;

	kfd->vm_info.first_vmid_kfd = ffs(gpu_resources->compute_vmid_bitmap)-1;
	kfd->vm_info.last_vmid_kfd = fls(gpu_resources->compute_vmid_bitmap)-1;
	kfd->vm_info.vmid_num_kfd = kfd->vm_info.last_vmid_kfd
			- kfd->vm_info.first_vmid_kfd + 1;

	/* Allow BIF to recode atomics to PCIe 3.0 AtomicOps.
	 * 32 and 64-bit requests are possible and must be
	 * supported.
	 */
	kfd->pci_atomic_requested = amdgpu_amdkfd_have_atomics_support(kfd->adev);
	if (!kfd->pci_atomic_requested &&
	    kfd->device_info.needs_pci_atomics &&
	    (!kfd->device_info.no_atomic_fw_version ||
	     kfd->mec_fw_version < kfd->device_info.no_atomic_fw_version)) {
		dev_info(kfd_device,
			 "skipped device %x:%x, PCI rejects atomics %d<%d\n",
			 kfd->adev->pdev->vendor, kfd->adev->pdev->device,
			 kfd->mec_fw_version,
			 kfd->device_info.no_atomic_fw_version);
		return false;
	}

	/* Verify module parameters regarding mapped process number*/
	if (hws_max_conc_proc >= 0)
		kfd->max_proc_per_quantum = min((u32)hws_max_conc_proc, kfd->vm_info.vmid_num_kfd);
	else
		kfd->max_proc_per_quantum = kfd->vm_info.vmid_num_kfd;

	/* calculate max size of mqds needed for queues */
	size = max_num_of_queues_per_device *
			kfd->device_info.mqd_size_aligned;

	/*
	 * calculate max size of runlist packet.
	 * There can be only 2 packets at once
	 */
	map_process_packet_size = KFD_GC_VERSION(kfd) == IP_VERSION(9, 4, 2) ?
				sizeof(struct pm4_mes_map_process_aldebaran) :
				sizeof(struct pm4_mes_map_process);
	size += (KFD_MAX_NUM_OF_PROCESSES * map_process_packet_size +
		max_num_of_queues_per_device * sizeof(struct pm4_mes_map_queues)
		+ sizeof(struct pm4_mes_runlist)) * 2;

	/* Add size of HIQ & DIQ */
	size += KFD_KERNEL_QUEUE_SIZE * 2;

	/* add another 512KB for all other allocations on gart (HPD, fences) */
	size += 512 * 1024;

	if (amdgpu_amdkfd_alloc_gtt_mem(
			kfd->adev, size, &kfd->gtt_mem,
			&kfd->gtt_start_gpu_addr, &kfd->gtt_start_cpu_ptr,
			false)) {
		dev_err(kfd_device, "Could not allocate %d bytes\n", size);
		goto alloc_gtt_mem_failure;
	}

	dev_info(kfd_device, "Allocated %d bytes on gart\n", size);

	/* Initialize GTT sa with 512 byte chunk size */
	if (kfd_gtt_sa_init(kfd, size, 512) != 0) {
		dev_err(kfd_device, "Error initializing gtt sub-allocator\n");
		goto kfd_gtt_sa_init_error;
	}

	if (kfd_doorbell_init(kfd)) {
		dev_err(kfd_device,
			"Error initializing doorbell aperture\n");
		goto kfd_doorbell_error;
	}

	if (amdgpu_use_xgmi_p2p)
		kfd->hive_id = kfd->adev->gmc.xgmi.hive_id;

	kfd->noretry = kfd->adev->gmc.noretry;

	if (kfd_interrupt_init(kfd)) {
		dev_err(kfd_device, "Error initializing interrupts\n");
		goto kfd_interrupt_error;
	}

	kfd->dqm = device_queue_manager_init(kfd);
	if (!kfd->dqm) {
		dev_err(kfd_device, "Error initializing queue manager\n");
		goto device_queue_manager_error;
	}

	/* If supported on this device, allocate global GWS that is shared
	 * by all KFD processes
	 */
	if (kfd_gws_init(kfd)) {
		dev_err(kfd_device, "Could not allocate %d gws\n",
			kfd->adev->gds.gws_size);
		goto gws_error;
	}

	/* If CRAT is broken, won't set iommu enabled */
	kfd_double_confirm_iommu_support(kfd);

	if (kfd_iommu_device_init(kfd)) {
		kfd->use_iommu_v2 = false;
		dev_err(kfd_device, "Error initializing iommuv2\n");
		goto device_iommu_error;
	}

	kfd_cwsr_init(kfd);

	svm_migrate_init(kfd->adev);

	if (kgd2kfd_resume_iommu(kfd))
		goto device_iommu_error;

	if (kfd_resume(kfd))
		goto kfd_resume_error;

	amdgpu_amdkfd_get_local_mem_info(kfd->adev, &kfd->local_mem_info);

	if (kfd_topology_add_device(kfd)) {
		dev_err(kfd_device, "Error adding device to topology\n");
		goto kfd_topology_add_device_error;
	}

	kfd_smi_init(kfd);

	kfd->init_complete = true;
	dev_info(kfd_device, "added device %x:%x\n", kfd->adev->pdev->vendor,
		 kfd->adev->pdev->device);

	pr_debug("Starting kfd with the following scheduling policy %d\n",
		kfd->dqm->sched_policy);

	goto out;

kfd_topology_add_device_error:
kfd_resume_error:
device_iommu_error:
gws_error:
	device_queue_manager_uninit(kfd->dqm);
device_queue_manager_error:
	kfd_interrupt_exit(kfd);
kfd_interrupt_error:
	kfd_doorbell_fini(kfd);
kfd_doorbell_error:
	kfd_gtt_sa_fini(kfd);
kfd_gtt_sa_init_error:
	amdgpu_amdkfd_free_gtt_mem(kfd->adev, kfd->gtt_mem);
alloc_gtt_mem_failure:
	if (kfd->gws)
		amdgpu_amdkfd_free_gws(kfd->adev, kfd->gws);
	dev_err(kfd_device,
		"device %x:%x NOT added due to errors\n",
		kfd->adev->pdev->vendor, kfd->adev->pdev->device);
out:
	return kfd->init_complete;
}

struct kgd2kfd_device_init__HEXSHA { void _d69a3b762dc4; };
struct kgd2kfd_device_init__ATTRS { };
