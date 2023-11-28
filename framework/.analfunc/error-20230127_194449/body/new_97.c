static struct hdmi *msm_hdmi_init(struct platform_device *pdev)
{
	struct hdmi_platform_config *config = pdev->dev.platform_data;
	struct hdmi *hdmi = NULL;
	struct resource *res;
	int i, ret;

	hdmi = devm_kzalloc(&pdev->dev, sizeof(*hdmi), GFP_KERNEL);
	if (!hdmi) {
		ret = -ENOMEM;
		goto fail;
	}

	hdmi->pdev = pdev;
	hdmi->config = config;
	spin_lock_init(&hdmi->reg_lock);

	ret = drm_of_find_panel_or_bridge(pdev->dev.of_node, 1, 0, NULL, &hdmi->next_bridge);
	if (ret && ret != -ENODEV)
		goto fail;

	hdmi->mmio = msm_ioremap(pdev, config->mmio_name);
	if (IS_ERR(hdmi->mmio)) {
		ret = PTR_ERR(hdmi->mmio);
		goto fail;
	}

	/* HDCP needs physical address of hdmi register */
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
		config->mmio_name);
	if (!res) {
		ret = -EINVAL;
		goto fail;
	}
	hdmi->mmio_phy_addr = res->start;

	hdmi->qfprom_mmio = msm_ioremap(pdev, config->qfprom_mmio_name);
	if (IS_ERR(hdmi->qfprom_mmio)) {
		DRM_DEV_INFO(&pdev->dev, "can't find qfprom resource\n");
		hdmi->qfprom_mmio = NULL;
	}

	hdmi->hpd_regs = devm_kcalloc(&pdev->dev,
				      config->hpd_reg_cnt,
				      sizeof(hdmi->hpd_regs[0]),
				      GFP_KERNEL);
	if (!hdmi->hpd_regs) {
		ret = -ENOMEM;
		goto fail;
	}
	for (i = 0; i < config->hpd_reg_cnt; i++)
		hdmi->hpd_regs[i].supply = config->hpd_reg_names[i];

	ret = devm_regulator_bulk_get(&pdev->dev, config->hpd_reg_cnt, hdmi->hpd_regs);
	if (ret) {
		DRM_DEV_ERROR(&pdev->dev, "failed to get hpd regulator: %d\n", ret);
		goto fail;
	}

	hdmi->pwr_regs = devm_kcalloc(&pdev->dev,
				      config->pwr_reg_cnt,
				      sizeof(hdmi->pwr_regs[0]),
				      GFP_KERNEL);
	if (!hdmi->pwr_regs) {
		ret = -ENOMEM;
		goto fail;
	}

	for (i = 0; i < config->pwr_reg_cnt; i++)
		hdmi->pwr_regs[i].supply = config->pwr_reg_names[i];

	ret = devm_regulator_bulk_get(&pdev->dev, config->pwr_reg_cnt, hdmi->pwr_regs);
	if (ret) {
		DRM_DEV_ERROR(&pdev->dev, "failed to get pwr regulator: %d\n", ret);
		goto fail;
	}

	hdmi->hpd_clks = devm_kcalloc(&pdev->dev,
				      config->hpd_clk_cnt,
				      sizeof(hdmi->hpd_clks[0]),
				      GFP_KERNEL);
	if (!hdmi->hpd_clks) {
		ret = -ENOMEM;
		goto fail;
	}
	for (i = 0; i < config->hpd_clk_cnt; i++) {
		struct clk *clk;

		clk = msm_clk_get(pdev, config->hpd_clk_names[i]);
		if (IS_ERR(clk)) {
			ret = PTR_ERR(clk);
			DRM_DEV_ERROR(&pdev->dev, "failed to get hpd clk: %s (%d)\n",
					config->hpd_clk_names[i], ret);
			goto fail;
		}

		hdmi->hpd_clks[i] = clk;
	}

	hdmi->pwr_clks = devm_kcalloc(&pdev->dev,
				      config->pwr_clk_cnt,
				      sizeof(hdmi->pwr_clks[0]),
				      GFP_KERNEL);
	if (!hdmi->pwr_clks) {
		ret = -ENOMEM;
		goto fail;
	}
	for (i = 0; i < config->pwr_clk_cnt; i++) {
		struct clk *clk;

		clk = msm_clk_get(pdev, config->pwr_clk_names[i]);
		if (IS_ERR(clk)) {
			ret = PTR_ERR(clk);
			DRM_DEV_ERROR(&pdev->dev, "failed to get pwr clk: %s (%d)\n",
					config->pwr_clk_names[i], ret);
			goto fail;
		}

		hdmi->pwr_clks[i] = clk;
	}

	hdmi->hpd_gpiod = devm_gpiod_get_optional(&pdev->dev, "hpd", GPIOD_IN);
	/* This will catch e.g. -EPROBE_DEFER */
	if (IS_ERR(hdmi->hpd_gpiod)) {
		ret = PTR_ERR(hdmi->hpd_gpiod);
		DRM_DEV_ERROR(&pdev->dev, "failed to get hpd gpio: (%d)\n", ret);
		goto fail;
	}

	if (!hdmi->hpd_gpiod)
		DBG("failed to get HPD gpio");

	if (hdmi->hpd_gpiod)
		gpiod_set_consumer_name(hdmi->hpd_gpiod, "HDMI_HPD");

	devm_pm_runtime_enable(&pdev->dev);

	hdmi->workq = alloc_ordered_workqueue("msm_hdmi", 0);

	hdmi->i2c = msm_hdmi_i2c_init(hdmi);
	if (IS_ERR(hdmi->i2c)) {
		ret = PTR_ERR(hdmi->i2c);
		DRM_DEV_ERROR(&pdev->dev, "failed to get i2c: %d\n", ret);
		hdmi->i2c = NULL;
		goto fail;
	}

	ret = msm_hdmi_get_phy(hdmi);
	if (ret) {
		DRM_DEV_ERROR(&pdev->dev, "failed to get phy\n");
		goto fail;
	}

	hdmi->hdcp_ctrl = msm_hdmi_hdcp_init(hdmi);
	if (IS_ERR(hdmi->hdcp_ctrl)) {
		dev_warn(&pdev->dev, "failed to init hdcp: disabled\n");
		hdmi->hdcp_ctrl = NULL;
	}

	return hdmi;

fail:
	if (hdmi)
		msm_hdmi_destroy(hdmi);

	return ERR_PTR(ret);
}

struct msm_hdmi_init__HEXSHA { void _b964444b2b64; };
struct msm_hdmi_init__ATTRS { };
