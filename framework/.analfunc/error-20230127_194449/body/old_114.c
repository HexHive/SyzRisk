struct kfd_process_device *kfd_bind_process_to_device(struct kfd_dev *dev,
							struct kfd_process *p)
{
	struct kfd_process_device *pdd;
	int err;

	pdd = kfd_get_process_device_data(dev, p);
	if (!pdd) {
		pr_err("Process device data doesn't exist\n");
		return ERR_PTR(-ENOMEM);
	}

	if (!pdd->drm_priv)
		return ERR_PTR(-ENODEV);

	/*
	 * signal runtime-pm system to auto resume and prevent
	 * further runtime suspend once device pdd is created until
	 * pdd is destroyed.
	 */
	if (!pdd->runtime_inuse) {
		err = pm_runtime_get_sync(dev->ddev->dev);
		if (err < 0) {
			pm_runtime_put_autosuspend(dev->ddev->dev);
			return ERR_PTR(err);
		}
	}

	err = kfd_iommu_bind_process_to_device(pdd);
	if (err)
		goto out;

	/*
	 * make sure that runtime_usage counter is incremented just once
	 * per pdd
	 */
	pdd->runtime_inuse = true;

	return pdd;

out:
	/* balance runpm reference count and exit with error */
	if (!pdd->runtime_inuse) {
		pm_runtime_mark_last_busy(dev->ddev->dev);
		pm_runtime_put_autosuspend(dev->ddev->dev);
	}

	return ERR_PTR(err);
}

struct kfd_bind_process_to_device__HEXSHA { void _d69a3b762dc4; };
struct kfd_bind_process_to_device__ATTRS { };
