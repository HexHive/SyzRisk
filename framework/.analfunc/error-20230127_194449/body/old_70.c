static u64 vp_vdpa_get_device_features(struct vdpa_device *vdpa)
{
	struct virtio_pci_modern_device *mdev = vdpa_to_mdev(vdpa);

	return vp_modern_get_features(mdev);
}

struct vp_vdpa_get_device_features__HEXSHA { void _c1ca352d371f; };
struct vp_vdpa_get_device_features__ATTRS { };
