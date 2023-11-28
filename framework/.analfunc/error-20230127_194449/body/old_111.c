int kfd_topology_add_device(struct kfd_dev *gpu)
{
	uint32_t gpu_id;
	struct kfd_topology_device *dev;
	struct kfd_cu_info cu_info;
	int res = 0;
	struct list_head temp_topology_device_list;
	void *crat_image = NULL;
	size_t image_size = 0;
	int proximity_domain;
	int i;
	const char *asic_name = amdgpu_asic_name[gpu->adev->asic_type];

	INIT_LIST_HEAD(&temp_topology_device_list);

	gpu_id = kfd_generate_gpu_id(gpu);
	pr_debug("Adding new GPU (ID: 0x%x) to topology\n", gpu_id);

	/* Check to see if this gpu device exists in the topology_device_list.
	 * If so, assign the gpu to that device,
	 * else create a Virtual CRAT for this gpu device and then parse that
	 * CRAT to create a new topology device. Once created assign the gpu to
	 * that topology device
	 */
	dev = kfd_assign_gpu(gpu);
	if (!dev) {
		down_write(&topology_lock);
		proximity_domain = ++topology_crat_proximity_domain;

		res = kfd_create_crat_image_virtual(&crat_image, &image_size,
						    COMPUTE_UNIT_GPU, gpu,
						    proximity_domain);
		if (res) {
			pr_err("Error creating VCRAT for GPU (ID: 0x%x)\n",
			       gpu_id);
			topology_crat_proximity_domain--;
			return res;
		}
		res = kfd_parse_crat_table(crat_image,
					   &temp_topology_device_list,
					   proximity_domain);
		if (res) {
			pr_err("Error parsing VCRAT for GPU (ID: 0x%x)\n",
			       gpu_id);
			topology_crat_proximity_domain--;
			goto err;
		}

		kfd_topology_update_device_list(&temp_topology_device_list,
			&topology_device_list);

		/* Update the SYSFS tree, since we added another topology
		 * device
		 */
		res = kfd_topology_update_sysfs();
		up_write(&topology_lock);

		if (!res)
			sys_props.generation_count++;
		else
			pr_err("Failed to update GPU (ID: 0x%x) to sysfs topology. res=%d\n",
						gpu_id, res);
		dev = kfd_assign_gpu(gpu);
		if (WARN_ON(!dev)) {
			res = -ENODEV;
			goto err;
		}
	}

	dev->gpu_id = gpu_id;
	gpu->id = gpu_id;

	kfd_dev_create_p2p_links();

	/* TODO: Move the following lines to function
	 *	kfd_add_non_crat_information
	 */

	/* Fill-in additional information that is not available in CRAT but
	 * needed for the topology
	 */

	amdgpu_amdkfd_get_cu_info(dev->gpu->adev, &cu_info);

	for (i = 0; i < KFD_TOPOLOGY_PUBLIC_NAME_SIZE-1; i++) {
		dev->node_props.name[i] = __tolower(asic_name[i]);
		if (asic_name[i] == '\0')
			break;
	}
	dev->node_props.name[i] = '\0';

	dev->node_props.simd_arrays_per_engine =
		cu_info.num_shader_arrays_per_engine;

	dev->node_props.gfx_target_version = gpu->device_info.gfx_target_version;
	dev->node_props.vendor_id = gpu->pdev->vendor;
	dev->node_props.device_id = gpu->pdev->device;
	dev->node_props.capability |=
		((dev->gpu->adev->rev_id << HSA_CAP_ASIC_REVISION_SHIFT) &
			HSA_CAP_ASIC_REVISION_MASK);
	dev->node_props.location_id = pci_dev_id(gpu->pdev);
	dev->node_props.domain = pci_domain_nr(gpu->pdev->bus);
	dev->node_props.max_engine_clk_fcompute =
		amdgpu_amdkfd_get_max_engine_clock_in_mhz(dev->gpu->adev);
	dev->node_props.max_engine_clk_ccompute =
		cpufreq_quick_get_max(0) / 1000;
	dev->node_props.drm_render_minor =
		gpu->shared_resources.drm_render_minor;

	dev->node_props.hive_id = gpu->hive_id;
	dev->node_props.num_sdma_engines = kfd_get_num_sdma_engines(gpu);
	dev->node_props.num_sdma_xgmi_engines =
					kfd_get_num_xgmi_sdma_engines(gpu);
	dev->node_props.num_sdma_queues_per_engine =
				gpu->device_info.num_sdma_queues_per_engine -
				gpu->device_info.num_reserved_sdma_queues_per_engine;
	dev->node_props.num_gws = (dev->gpu->gws &&
		dev->gpu->dqm->sched_policy != KFD_SCHED_POLICY_NO_HWS) ?
		dev->gpu->adev->gds.gws_size : 0;
	dev->node_props.num_cp_queues = get_cp_queues_num(dev->gpu->dqm);

	kfd_fill_mem_clk_max_info(dev);
	kfd_fill_iolink_non_crat_info(dev);

	switch (dev->gpu->adev->asic_type) {
	case CHIP_KAVERI:
	case CHIP_HAWAII:
	case CHIP_TONGA:
		dev->node_props.capability |= ((HSA_CAP_DOORBELL_TYPE_PRE_1_0 <<
			HSA_CAP_DOORBELL_TYPE_TOTALBITS_SHIFT) &
			HSA_CAP_DOORBELL_TYPE_TOTALBITS_MASK);
		break;
	case CHIP_CARRIZO:
	case CHIP_FIJI:
	case CHIP_POLARIS10:
	case CHIP_POLARIS11:
	case CHIP_POLARIS12:
	case CHIP_VEGAM:
		pr_debug("Adding doorbell packet type capability\n");
		dev->node_props.capability |= ((HSA_CAP_DOORBELL_TYPE_1_0 <<
			HSA_CAP_DOORBELL_TYPE_TOTALBITS_SHIFT) &
			HSA_CAP_DOORBELL_TYPE_TOTALBITS_MASK);
		break;
	default:
		if (KFD_GC_VERSION(dev->gpu) >= IP_VERSION(9, 0, 1))
			dev->node_props.capability |= ((HSA_CAP_DOORBELL_TYPE_2_0 <<
				HSA_CAP_DOORBELL_TYPE_TOTALBITS_SHIFT) &
				HSA_CAP_DOORBELL_TYPE_TOTALBITS_MASK);
		else
			WARN(1, "Unexpected ASIC family %u",
			     dev->gpu->adev->asic_type);
	}

	/*
	 * Overwrite ATS capability according to needs_iommu_device to fix
	 * potential missing corresponding bit in CRAT of BIOS.
	 */
	if (dev->gpu->use_iommu_v2)
		dev->node_props.capability |= HSA_CAP_ATS_PRESENT;
	else
		dev->node_props.capability &= ~HSA_CAP_ATS_PRESENT;

	/* Fix errors in CZ CRAT.
	 * simd_count: Carrizo CRAT reports wrong simd_count, probably
	 *		because it doesn't consider masked out CUs
	 * max_waves_per_simd: Carrizo reports wrong max_waves_per_simd
	 */
	if (dev->gpu->adev->asic_type == CHIP_CARRIZO) {
		dev->node_props.simd_count =
			cu_info.simd_per_cu * cu_info.cu_active_number;
		dev->node_props.max_waves_per_simd = 10;
	}

	/* kfd only concerns sram ecc on GFX and HBM ecc on UMC */
	dev->node_props.capability |=
		((dev->gpu->adev->ras_enabled & BIT(AMDGPU_RAS_BLOCK__GFX)) != 0) ?
		HSA_CAP_SRAM_EDCSUPPORTED : 0;
	dev->node_props.capability |=
		((dev->gpu->adev->ras_enabled & BIT(AMDGPU_RAS_BLOCK__UMC)) != 0) ?
		HSA_CAP_MEM_EDCSUPPORTED : 0;

	if (KFD_GC_VERSION(dev->gpu) != IP_VERSION(9, 0, 1))
		dev->node_props.capability |= (dev->gpu->adev->ras_enabled != 0) ?
			HSA_CAP_RASEVENTNOTIFY : 0;

	if (KFD_IS_SVM_API_SUPPORTED(dev->gpu->adev->kfd.dev))
		dev->node_props.capability |= HSA_CAP_SVMAPI_SUPPORTED;

	kfd_debug_print_topology();

	if (!res)
		kfd_notify_gpu_change(gpu_id, 1);
err:
	kfd_destroy_crat_image(crat_image);
	return res;
}

struct kfd_topology_add_device__HEXSHA { void _d69a3b762dc4; };
struct kfd_topology_add_device__ATTRS { };
