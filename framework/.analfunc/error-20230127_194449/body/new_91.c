static int vp_vdpa_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct vp_vdpa_mgmtdev *vp_vdpa_mgtdev = NULL;
	struct vdpa_mgmt_dev *mgtdev;
	struct device *dev = &pdev->dev;
	struct virtio_pci_modern_device *mdev = NULL;
	struct virtio_device_id *mdev_id = NULL;
	int err;

	vp_vdpa_mgtdev = kzalloc(sizeof(*vp_vdpa_mgtdev), GFP_KERNEL);
	if (!vp_vdpa_mgtdev)
		return -ENOMEM;

	mgtdev = &vp_vdpa_mgtdev->mgtdev;
	mgtdev->ops = &vp_vdpa_mdev_ops;
	mgtdev->device = dev;

	mdev = kzalloc(sizeof(struct virtio_pci_modern_device), GFP_KERNEL);
	if (!mdev) {
		err = -ENOMEM;
		goto mdev_err;
	}

	mdev_id = kzalloc(sizeof(struct virtio_device_id), GFP_KERNEL);
	if (!mdev_id) {
		err = -ENOMEM;
		goto mdev_id_err;
	}

	vp_vdpa_mgtdev->mdev = mdev;
	mdev->pci_dev = pdev;

	err = pcim_enable_device(pdev);
	if (err) {
		goto probe_err;
	}

	err = vp_modern_probe(mdev);
	if (err) {
		dev_err(&pdev->dev, "Failed to probe modern PCI device\n");
		goto probe_err;
	}

	mdev_id->device = mdev->id.device;
	mdev_id->vendor = mdev->id.vendor;
	mgtdev->id_table = mdev_id;
	mgtdev->max_supported_vqs = vp_modern_get_num_queues(mdev);
	mgtdev->supported_features = vp_modern_get_features(mdev);
	mgtdev->config_attr_mask = (1 << VDPA_ATTR_DEV_FEATURES);
	pci_set_master(pdev);
	pci_set_drvdata(pdev, vp_vdpa_mgtdev);

	err = vdpa_mgmtdev_register(mgtdev);
	if (err) {
		dev_err(&pdev->dev, "Failed to register vdpa mgmtdev device\n");
		goto register_err;
	}

	return 0;

register_err:
	vp_modern_remove(vp_vdpa_mgtdev->mdev);
probe_err:
	kfree(mdev_id);
mdev_id_err:
	kfree(mdev);
mdev_err:
	kfree(vp_vdpa_mgtdev);
	return err;
}

struct vp_vdpa_probe__HEXSHA { void _c1ca352d371f; };
struct vp_vdpa_probe__ATTRS { };
