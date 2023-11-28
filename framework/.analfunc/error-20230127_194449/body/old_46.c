static struct thermal_cooling_device *
__thermal_cooling_device_register(struct device_node *np,
				  const char *type, void *devdata,
				  const struct thermal_cooling_device_ops *ops)
{
	struct thermal_cooling_device *cdev;
	struct thermal_zone_device *pos = NULL;
	int id, ret;

	if (!ops || !ops->get_max_state || !ops->get_cur_state ||
	    !ops->set_cur_state)
		return ERR_PTR(-EINVAL);

	cdev = kzalloc(sizeof(*cdev), GFP_KERNEL);
	if (!cdev)
		return ERR_PTR(-ENOMEM);

	ret = ida_alloc(&thermal_cdev_ida, GFP_KERNEL);
	if (ret < 0)
		goto out_kfree_cdev;
	cdev->id = ret;
	id = ret;

	ret = dev_set_name(&cdev->device, "cooling_device%d", cdev->id);
	if (ret)
		goto out_ida_remove;

	cdev->type = kstrdup(type ? type : "", GFP_KERNEL);
	if (!cdev->type) {
		ret = -ENOMEM;
		goto out_ida_remove;
	}

	mutex_init(&cdev->lock);
	INIT_LIST_HEAD(&cdev->thermal_instances);
	cdev->np = np;
	cdev->ops = ops;
	cdev->updated = false;
	cdev->device.class = &thermal_class;
	cdev->devdata = devdata;

	if (cdev->ops->get_max_state(cdev, &cdev->max_state))
		goto out_kfree_type;

	thermal_cooling_device_setup_sysfs(cdev);
	ret = device_register(&cdev->device);
	if (ret)
		goto out_kfree_type;

	/* Add 'this' new cdev to the global cdev list */
	mutex_lock(&thermal_list_lock);
	list_add(&cdev->node, &thermal_cdev_list);
	mutex_unlock(&thermal_list_lock);

	/* Update binding information for 'this' new cdev */
	bind_cdev(cdev);

	mutex_lock(&thermal_list_lock);
	list_for_each_entry(pos, &thermal_tz_list, node)
		if (atomic_cmpxchg(&pos->need_update, 1, 0))
			thermal_zone_device_update(pos,
						   THERMAL_EVENT_UNSPECIFIED);
	mutex_unlock(&thermal_list_lock);

	return cdev;

out_kfree_type:
	thermal_cooling_device_destroy_sysfs(cdev);
	kfree(cdev->type);
	put_device(&cdev->device);
	cdev = NULL;
out_ida_remove:
	ida_free(&thermal_cdev_ida, id);
out_kfree_cdev:
	kfree(cdev);
	return ERR_PTR(ret);
}

struct __thermal_cooling_device_register__HEXSHA { void _e49a1e1ee078; };
struct __thermal_cooling_device_register__ATTRS { };
