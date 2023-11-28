int i915_driver_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
	struct drm_i915_private *i915;
	int ret;

	i915 = i915_driver_create(pdev, ent);
	if (IS_ERR(i915))
		return PTR_ERR(i915);

	/* Disable nuclear pageflip by default on pre-ILK */
	if (!i915->params.nuclear_pageflip && DISPLAY_VER(i915) < 5)
		i915->drm.driver_features &= ~DRIVER_ATOMIC;

	ret = pci_enable_device(pdev);
	if (ret)
		goto out_fini;

	ret = i915_driver_early_probe(i915);
	if (ret < 0)
		goto out_pci_disable;

	disable_rpm_wakeref_asserts(&i915->runtime_pm);

	intel_vgpu_detect(i915);

	ret = intel_gt_probe_all(i915);
	if (ret < 0)
		goto out_runtime_pm_put;

	ret = i915_driver_mmio_probe(i915);
	if (ret < 0)
		goto out_tiles_cleanup;

	ret = i915_driver_hw_probe(i915);
	if (ret < 0)
		goto out_cleanup_mmio;

	ret = intel_modeset_init_noirq(i915);
	if (ret < 0)
		goto out_cleanup_hw;

	ret = intel_irq_install(i915);
	if (ret)
		goto out_cleanup_modeset;

	ret = intel_modeset_init_nogem(i915);
	if (ret)
		goto out_cleanup_irq;

	ret = i915_gem_init(i915);
	if (ret)
		goto out_cleanup_modeset2;

	ret = intel_modeset_init(i915);
	if (ret)
		goto out_cleanup_gem;

	i915_driver_register(i915);

	enable_rpm_wakeref_asserts(&i915->runtime_pm);

	i915_welcome_messages(i915);

	i915->do_release = true;

	return 0;

out_cleanup_gem:
	i915_gem_suspend(i915);
	i915_gem_driver_remove(i915);
	i915_gem_driver_release(i915);
out_cleanup_modeset2:
	/* FIXME clean up the error path */
	intel_modeset_driver_remove(i915);
	intel_irq_uninstall(i915);
	intel_modeset_driver_remove_noirq(i915);
	goto out_cleanup_modeset;
out_cleanup_irq:
	intel_irq_uninstall(i915);
out_cleanup_modeset:
	intel_modeset_driver_remove_nogem(i915);
out_cleanup_hw:
	i915_driver_hw_remove(i915);
	intel_memory_regions_driver_release(i915);
	i915_ggtt_driver_release(i915);
	i915_gem_drain_freed_objects(i915);
	i915_ggtt_driver_late_release(i915);
out_cleanup_mmio:
	i915_driver_mmio_release(i915);
out_tiles_cleanup:
	intel_gt_release_all(i915);
out_runtime_pm_put:
	enable_rpm_wakeref_asserts(&i915->runtime_pm);
	i915_driver_late_release(i915);
out_pci_disable:
	pci_disable_device(pdev);
out_fini:
	i915_probe_error(i915, "Device initialization failed (%d)\n", ret);
	return ret;
}

struct i915_driver_probe__HEXSHA { void _f38f614fa995; };
struct i915_driver_probe__ATTRS { };
