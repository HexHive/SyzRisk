static int vop_enable(struct drm_crtc *crtc, struct drm_crtc_state *old_state)
{
	struct vop *vop = to_vop(crtc);
	int ret, i;

	ret = pm_runtime_get_sync(vop->dev);
	if (ret < 0) {
		DRM_DEV_ERROR(vop->dev, "failed to get pm runtime: %d\n", ret);
		return ret;
	}

	ret = vop_core_clks_enable(vop);
	if (WARN_ON(ret < 0))
		goto err_put_pm_runtime;

	ret = clk_enable(vop->dclk);
	if (WARN_ON(ret < 0))
		goto err_disable_core;

	/*
	 * Slave iommu shares power, irq and clock with vop.  It was associated
	 * automatically with this master device via common driver code.
	 * Now that we have enabled the clock we attach it to the shared drm
	 * mapping.
	 */
	ret = rockchip_drm_dma_attach_device(vop->drm_dev, vop->dev);
	if (ret) {
		DRM_DEV_ERROR(vop->dev,
			      "failed to attach dma mapping, %d\n", ret);
		goto err_disable_dclk;
	}

	spin_lock(&vop->reg_lock);
	for (i = 0; i < vop->len; i += 4)
		writel_relaxed(vop->regsbak[i / 4], vop->regs + i);

	/*
	 * We need to make sure that all windows are disabled before we
	 * enable the crtc. Otherwise we might try to scan from a destroyed
	 * buffer later.
	 *
	 * In the case of enable-after-PSR, we don't need to worry about this
	 * case since the buffer is guaranteed to be valid and disabling the
	 * window will result in screen glitches on PSR exit.
	 */
	if (!old_state || !old_state->self_refresh_active) {
		for (i = 0; i < vop->data->win_size; i++) {
			struct vop_win *vop_win = &vop->win[i];

			vop_win_disable(vop, vop_win);
		}
	}

	if (vop->data->afbc) {
		struct rockchip_crtc_state *s;
		/*
		 * Disable AFBC and forget there was a vop window with AFBC
		 */
		VOP_AFBC_SET(vop, enable, 0);
		s = to_rockchip_crtc_state(crtc->state);
		s->enable_afbc = false;
	}

	vop_cfg_done(vop);

	spin_unlock(&vop->reg_lock);

	/*
	 * At here, vop clock & iommu is enable, R/W vop regs would be safe.
	 */
	vop->is_enabled = true;

	spin_lock(&vop->reg_lock);

	VOP_REG_SET(vop, common, standby, 1);

	spin_unlock(&vop->reg_lock);

	drm_crtc_vblank_on(crtc);

	return 0;

err_disable_dclk:
	clk_disable(vop->dclk);
err_disable_core:
	vop_core_clks_disable(vop);
err_put_pm_runtime:
	pm_runtime_put_sync(vop->dev);
	return ret;
}

struct vop_enable__HEXSHA { void _e3558747ebe1; };
struct vop_enable__ATTRS { };
