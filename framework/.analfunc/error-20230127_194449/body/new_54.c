static int dw_hdmi_rockchip_bind(struct device *dev, struct device *master,
				 void *data)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct dw_hdmi_plat_data *plat_data;
	const struct of_device_id *match;
	struct drm_device *drm = data;
	struct drm_encoder *encoder;
	struct rockchip_hdmi *hdmi;
	int ret;

	if (!pdev->dev.of_node)
		return -ENODEV;

	hdmi = devm_kzalloc(&pdev->dev, sizeof(*hdmi), GFP_KERNEL);
	if (!hdmi)
		return -ENOMEM;

	match = of_match_node(dw_hdmi_rockchip_dt_ids, pdev->dev.of_node);
	plat_data = devm_kmemdup(&pdev->dev, match->data,
					     sizeof(*plat_data), GFP_KERNEL);
	if (!plat_data)
		return -ENOMEM;

	hdmi->dev = &pdev->dev;
	hdmi->chip_data = plat_data->phy_data;
	plat_data->phy_data = hdmi;
	encoder = &hdmi->encoder.encoder;

	encoder->possible_crtcs = drm_of_find_possible_crtcs(drm, dev->of_node);
	rockchip_drm_encoder_set_crtc_endpoint_id(&hdmi->encoder,
						  dev->of_node, 0, 0);

	/*
	 * If we failed to find the CRTC(s) which this encoder is
	 * supposed to be connected to, it's because the CRTC has
	 * not been registered yet.  Defer probing, and hope that
	 * the required CRTC is added later.
	 */
	if (encoder->possible_crtcs == 0)
		return -EPROBE_DEFER;

	ret = rockchip_hdmi_parse_dt(hdmi);
	if (ret) {
		if (ret != -EPROBE_DEFER)
			DRM_DEV_ERROR(hdmi->dev, "Unable to parse OF data\n");
		return ret;
	}

	hdmi->phy = devm_phy_optional_get(dev, "hdmi");
	if (IS_ERR(hdmi->phy)) {
		ret = PTR_ERR(hdmi->phy);
		if (ret != -EPROBE_DEFER)
			DRM_DEV_ERROR(hdmi->dev, "failed to get phy\n");
		return ret;
	}

	ret = regulator_enable(hdmi->avdd_0v9);
	if (ret) {
		DRM_DEV_ERROR(hdmi->dev, "failed to enable avdd0v9: %d\n", ret);
		goto err_avdd_0v9;
	}

	ret = regulator_enable(hdmi->avdd_1v8);
	if (ret) {
		DRM_DEV_ERROR(hdmi->dev, "failed to enable avdd1v8: %d\n", ret);
		goto err_avdd_1v8;
	}

	ret = clk_prepare_enable(hdmi->ref_clk);
	if (ret) {
		DRM_DEV_ERROR(hdmi->dev, "Failed to enable HDMI reference clock: %d\n",
			      ret);
		goto err_clk;
	}

	if (hdmi->chip_data == &rk3568_chip_data) {
		regmap_write(hdmi->regmap, RK3568_GRF_VO_CON1,
			     HIWORD_UPDATE(RK3568_HDMI_SDAIN_MSK |
					   RK3568_HDMI_SCLIN_MSK,
					   RK3568_HDMI_SDAIN_MSK |
					   RK3568_HDMI_SCLIN_MSK));
	}

	drm_encoder_helper_add(encoder, &dw_hdmi_rockchip_encoder_helper_funcs);
	drm_simple_encoder_init(drm, encoder, DRM_MODE_ENCODER_TMDS);

	platform_set_drvdata(pdev, hdmi);

	hdmi->hdmi = dw_hdmi_bind(pdev, encoder, plat_data);

	/*
	 * If dw_hdmi_bind() fails we'll never call dw_hdmi_unbind(),
	 * which would have called the encoder cleanup.  Do it manually.
	 */
	if (IS_ERR(hdmi->hdmi)) {
		ret = PTR_ERR(hdmi->hdmi);
		goto err_bind;
	}

	return 0;

err_bind:
	drm_encoder_cleanup(encoder);
	clk_disable_unprepare(hdmi->ref_clk);
err_clk:
	regulator_disable(hdmi->avdd_1v8);
err_avdd_1v8:
	regulator_disable(hdmi->avdd_0v9);
err_avdd_0v9:
	return ret;
}

struct dw_hdmi_rockchip_bind__HEXSHA { void _bfab00b94bd8; };
struct dw_hdmi_rockchip_bind__ATTRS { };
