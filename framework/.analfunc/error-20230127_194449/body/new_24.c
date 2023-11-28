static void vop2_enable(struct vop2 *vop2)
{
	int ret;

	ret = pm_runtime_resume_and_get(vop2->dev);
	if (ret < 0) {
		drm_err(vop2->drm, "failed to get pm runtime: %d\n", ret);
		return;
	}

	ret = vop2_core_clks_prepare_enable(vop2);
	if (ret) {
		pm_runtime_put_sync(vop2->dev);
		return;
	}

	ret = rockchip_drm_dma_attach_device(vop2->drm, vop2->dev);
	if (ret) {
		drm_err(vop2->drm, "failed to attach dma mapping, %d\n", ret);
		return;
	}

	if (vop2->data->soc_id == 3566)
		vop2_writel(vop2, RK3568_OTP_WIN_EN, 1);

	vop2_writel(vop2, RK3568_REG_CFG_DONE, RK3568_REG_CFG_DONE__GLB_CFG_DONE_EN);

	/*
	 * Disable auto gating, this is a workaround to
	 * avoid display image shift when a window enabled.
	 */
	regmap_clear_bits(vop2->map, RK3568_SYS_AUTO_GATING_CTRL,
			  RK3568_SYS_AUTO_GATING_CTRL__AUTO_GATING_EN);

	vop2_writel(vop2, RK3568_SYS0_INT_CLR,
		    VOP2_INT_BUS_ERRPR << 16 | VOP2_INT_BUS_ERRPR);
	vop2_writel(vop2, RK3568_SYS0_INT_EN,
		    VOP2_INT_BUS_ERRPR << 16 | VOP2_INT_BUS_ERRPR);
	vop2_writel(vop2, RK3568_SYS1_INT_CLR,
		    VOP2_INT_BUS_ERRPR << 16 | VOP2_INT_BUS_ERRPR);
	vop2_writel(vop2, RK3568_SYS1_INT_EN,
		    VOP2_INT_BUS_ERRPR << 16 | VOP2_INT_BUS_ERRPR);
}

struct vop2_enable__HEXSHA { void _e3558747ebe1; };
struct vop2_enable__ATTRS { };
