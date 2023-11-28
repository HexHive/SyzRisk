void amdgpu_amdkfd_device_init(struct amdgpu_device *adev)
{
	int i;
	int last_valid_bit;

	if (adev->kfd.dev) {
		struct kgd2kfd_shared_resources gpu_resources = {
			.compute_vmid_bitmap =
				((1 << AMDGPU_NUM_VMID) - 1) -
				((1 << adev->vm_manager.first_kfd_vmid) - 1),
			.num_pipe_per_mec = adev->gfx.mec.num_pipe_per_mec,
			.num_queue_per_pipe = adev->gfx.mec.num_queue_per_pipe,
			.gpuvm_size = min(adev->vm_manager.max_pfn
					  << AMDGPU_GPU_PAGE_SHIFT,
					  AMDGPU_GMC_HOLE_START),
			.drm_render_minor = adev_to_drm(adev)->render->index,
			.sdma_doorbell_idx = adev->doorbell_index.sdma_engine,
			.enable_mes = adev->enable_mes,
		};

		/* this is going to have a few of the MSBs set that we need to
		 * clear
		 */
		bitmap_complement(gpu_resources.cp_queue_bitmap,
				  adev->gfx.mec.queue_bitmap,
				  KGD_MAX_QUEUES);

		/* According to linux/bitmap.h we shouldn't use bitmap_clear if
		 * nbits is not compile time constant
		 */
		last_valid_bit = 1 /* only first MEC can have compute queues */
				* adev->gfx.mec.num_pipe_per_mec
				* adev->gfx.mec.num_queue_per_pipe;
		for (i = last_valid_bit; i < KGD_MAX_QUEUES; ++i)
			clear_bit(i, gpu_resources.cp_queue_bitmap);

		amdgpu_doorbell_get_kfd_info(adev,
				&gpu_resources.doorbell_physical_address,
				&gpu_resources.doorbell_aperture_size,
				&gpu_resources.doorbell_start_offset);

		/* Since SOC15, BIF starts to statically use the
		 * lower 12 bits of doorbell addresses for routing
		 * based on settings in registers like
		 * SDMA0_DOORBELL_RANGE etc..
		 * In order to route a doorbell to CP engine, the lower
		 * 12 bits of its address has to be outside the range
		 * set for SDMA, VCN, and IH blocks.
		 */
		if (adev->asic_type >= CHIP_VEGA10) {
			gpu_resources.non_cp_doorbells_start =
					adev->doorbell_index.first_non_cp;
			gpu_resources.non_cp_doorbells_end =
					adev->doorbell_index.last_non_cp;
		}

		adev->kfd.init_complete = kgd2kfd_device_init(adev->kfd.dev,
						adev_to_drm(adev), &gpu_resources);

		amdgpu_amdkfd_total_mem_size += adev->gmc.real_vram_size;

		INIT_WORK(&adev->kfd.reset_work, amdgpu_amdkfd_reset_work);
	}
}

struct amdgpu_amdkfd_device_init__HEXSHA { void _d69a3b762dc4; };
struct amdgpu_amdkfd_device_init__ATTRS { };
