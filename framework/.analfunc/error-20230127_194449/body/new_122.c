static inline int kfd_devcgroup_check_permission(struct kfd_dev *kfd)
{

	struct drm_device *ddev = adev_to_drm(kfd->adev);

	return devcgroup_check_permission(DEVCG_DEV_CHAR, DRM_MAJOR,
					  ddev->render->index,
					  DEVCG_ACC_WRITE | DEVCG_ACC_READ);



}

struct kfd_devcgroup_check_permission__HEXSHA { void _d69a3b762dc4; };
struct kfd_devcgroup_check_permission__ATTRS { };
