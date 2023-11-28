static int kfd_bind_processes_to_device(struct kfd_dev *kfd)
{
	struct kfd_process_device *pdd;
	struct kfd_process *p;
	unsigned int temp;
	int err = 0;

	int idx = srcu_read_lock(&kfd_processes_srcu);

	hash_for_each_rcu(kfd_processes_table, temp, p, kfd_processes) {
		mutex_lock(&p->mutex);
		pdd = kfd_get_process_device_data(kfd, p);

		if (WARN_ON(!pdd) || pdd->bound != PDD_BOUND_SUSPENDED) {
			mutex_unlock(&p->mutex);
			continue;
		}

		err = amd_iommu_bind_pasid(kfd->pdev, p->pasid,
				p->lead_thread);
		if (err < 0) {
			pr_err("Unexpected pasid 0x%x binding failure\n",
					p->pasid);
			mutex_unlock(&p->mutex);
			break;
		}

		pdd->bound = PDD_BOUND;
		mutex_unlock(&p->mutex);
	}

	srcu_read_unlock(&kfd_processes_srcu, idx);

	return err;
}

struct kfd_bind_processes_to_device__HEXSHA { void _d69a3b762dc4; };
struct kfd_bind_processes_to_device__ATTRS { };
