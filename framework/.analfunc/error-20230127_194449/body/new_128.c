static int kfd_resume(struct kfd_dev *kfd)
{
	int err = 0;

	err = kfd->dqm->ops.start(kfd->dqm);
	if (err)
		dev_err(kfd_device,
			"Error starting queue manager for device %x:%x\n",
			kfd->adev->pdev->vendor, kfd->adev->pdev->device);

	return err;
}

struct kfd_resume__HEXSHA { void _d69a3b762dc4; };
struct kfd_resume__ATTRS { };
