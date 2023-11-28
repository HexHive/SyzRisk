static void kfd_process_destroy_pdds(struct kfd_process *p)
{
	int i;

	for (i = 0; i < p->n_pdds; i++) {
		struct kfd_process_device *pdd = p->pdds[i];

		pr_debug("Releasing pdd (topology id %d) for process (pasid 0x%x)\n",
				pdd->dev->id, p->pasid);

		kfd_process_device_destroy_cwsr_dgpu(pdd);
		kfd_process_device_destroy_ib_mem(pdd);

		if (pdd->drm_file) {
			amdgpu_amdkfd_gpuvm_release_process_vm(
					pdd->dev->adev, pdd->drm_priv);
			fput(pdd->drm_file);
		}

		if (pdd->qpd.cwsr_kaddr && !pdd->qpd.cwsr_base)
			free_pages((unsigned long)pdd->qpd.cwsr_kaddr,
				get_order(KFD_CWSR_TBA_TMA_SIZE));

		bitmap_free(pdd->qpd.doorbell_bitmap);
		idr_destroy(&pdd->alloc_idr);

		kfd_free_process_doorbells(pdd->dev, pdd->doorbell_index);

		if (pdd->dev->shared_resources.enable_mes)
			amdgpu_amdkfd_free_gtt_mem(pdd->dev->adev,
						   pdd->proc_ctx_bo);
		/*
		 * before destroying pdd, make sure to report availability
		 * for auto suspend
		 */
		if (pdd->runtime_inuse) {
			pm_runtime_mark_last_busy(pdd->dev->ddev->dev);
			pm_runtime_put_autosuspend(pdd->dev->ddev->dev);
			pdd->runtime_inuse = false;
		}

		kfree(pdd);
		p->pdds[i] = NULL;
	}
	p->n_pdds = 0;
}

struct kfd_process_destroy_pdds__HEXSHA { void _d69a3b762dc4; };
struct kfd_process_destroy_pdds__ATTRS { };
