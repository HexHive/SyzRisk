static int vp_vdpa_dev_add(struct vdpa_mgmt_dev *v_mdev, const char *name,
			   const struct vdpa_dev_set_config *add_config)
{
	struct vp_vdpa_mgmtdev *vp_vdpa_mgtdev =
		container_of(v_mdev, struct vp_vdpa_mgmtdev, mgtdev);

	struct virtio_pci_modern_device *mdev = vp_vdpa_mgtdev->mdev;
	struct pci_dev *pdev = mdev->pci_dev;
	struct device *dev = &pdev->dev;
	struct vp_vdpa *vp_vdpa = NULL;
	u64 device_features;
	int ret, i;

	vp_vdpa = vdpa_alloc_device(struct vp_vdpa, vdpa,
				    dev, &vp_vdpa_ops, 1, 1, name, false);

	if (IS_ERR(vp_vdpa)) {
		dev_err(dev, "vp_vdpa: Failed to allocate vDPA structure\n");
		return PTR_ERR(vp_vdpa);
	}

	vp_vdpa_mgtdev->vp_vdpa = vp_vdpa;

	vp_vdpa->vdpa.dma_dev = &pdev->dev;
	vp_vdpa->queues = vp_modern_get_num_queues(mdev);
	vp_vdpa->mdev = mdev;

	device_features = vp_modern_get_features(mdev);
	if (add_config->mask & BIT_ULL(VDPA_ATTR_DEV_FEATURES)) {
		if (add_config->device_features & ~device_features) {
			ret = -EINVAL;
			dev_err(&pdev->dev, "Try to provision features "
				"that are not supported by the device: "
				"device_features 0x%llx provisioned 0x%llx\n",
				device_features, add_config->device_features);
			goto err;
		}
		device_features = add_config->device_features;
	}
	vp_vdpa->device_features = device_features;

	ret = devm_add_action_or_reset(dev, vp_vdpa_free_irq_vectors, pdev);
	if (ret) {
		dev_err(&pdev->dev,
			"Failed for adding devres for freeing irq vectors\n");
		goto err;
	}

	vp_vdpa->vring = devm_kcalloc(&pdev->dev, vp_vdpa->queues,
				      sizeof(*vp_vdpa->vring),
				      GFP_KERNEL);
	if (!vp_vdpa->vring) {
		ret = -ENOMEM;
		dev_err(&pdev->dev, "Fail to allocate virtqueues\n");
		goto err;
	}

	for (i = 0; i < vp_vdpa->queues; i++) {
		vp_vdpa->vring[i].irq = VIRTIO_MSI_NO_VECTOR;
		vp_vdpa->vring[i].notify =
			vp_modern_map_vq_notify(mdev, i,
						&vp_vdpa->vring[i].notify_pa);
		if (!vp_vdpa->vring[i].notify) {
			ret = -EINVAL;
			dev_warn(&pdev->dev, "Fail to map vq notify %d\n", i);
			goto err;
		}
	}
	vp_vdpa->config_irq = VIRTIO_MSI_NO_VECTOR;

	vp_vdpa->vdpa.mdev = &vp_vdpa_mgtdev->mgtdev;
	ret = _vdpa_register_device(&vp_vdpa->vdpa, vp_vdpa->queues);
	if (ret) {
		dev_err(&pdev->dev, "Failed to register to vdpa bus\n");
		goto err;
	}

	return 0;

err:
	put_device(&vp_vdpa->vdpa.dev);
	return ret;
}

struct vp_vdpa_dev_add__HEXSHA { void _c1ca352d371f; };
struct vp_vdpa_dev_add__ATTRS { };
