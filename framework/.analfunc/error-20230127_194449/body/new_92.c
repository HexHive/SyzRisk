static u64 vp_vdpa_get_device_features(struct vdpa_device *vdpa)
{
	struct vp_vdpa *vp_vdpa = vdpa_to_vp(vdpa);

	return vp_vdpa->device_features;
}

struct vp_vdpa_get_device_features__HEXSHA { void _c1ca352d371f; };
struct vp_vdpa_get_device_features__ATTRS { };
