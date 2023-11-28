static int dw_mipi_dsi_dphy_power_on(struct phy *phy)
{
	struct dw_mipi_dsi_rockchip *dsi = phy_get_drvdata(phy);
	int i, ret;

	DRM_DEV_DEBUG(dsi->dev, "lanes %d - data_rate_mbps %u\n",
		      dsi->dphy_config.lanes, dsi->lane_mbps);

	i = max_mbps_to_parameter(dsi->lane_mbps);
	if (i < 0) {
		DRM_DEV_ERROR(dsi->dev, "failed to get parameter for %dmbps clock\n",
			      dsi->lane_mbps);
		return i;
	}

	ret = pm_runtime_get_sync(dsi->dev);
	if (ret < 0) {
		DRM_DEV_ERROR(dsi->dev, "failed to enable device: %d\n", ret);
		return ret;
	}

	ret = clk_prepare_enable(dsi->pclk);
	if (ret) {
		DRM_DEV_ERROR(dsi->dev, "Failed to enable pclk: %d\n", ret);
		goto err_pclk;
	}

	ret = clk_prepare_enable(dsi->grf_clk);
	if (ret) {
		DRM_DEV_ERROR(dsi->dev, "Failed to enable grf_clk: %d\n", ret);
		goto err_grf_clk;
	}

	ret = clk_prepare_enable(dsi->phy_cfg_clk);
	if (ret) {
		DRM_DEV_ERROR(dsi->dev, "Failed to enable phy_cfg_clk: %d\n", ret);
		goto err_phy_cfg_clk;
	}

	/* do soc-variant specific init */
	if (dsi->cdata->dphy_rx_power_on) {
		ret = dsi->cdata->dphy_rx_power_on(phy);
		if (ret < 0) {
			DRM_DEV_ERROR(dsi->dev, "hardware-specific phy bringup failed: %d\n", ret);
			goto err_pwr_on;
		}
	}

	/*
	 * Configure hsfreqrange according to frequency values
	 * Set clock lane and hsfreqrange by lane0(test code 0x44)
	 */
	dw_mipi_dsi_phy_write(dsi, HS_RX_CONTROL_OF_LANE_CLK, 0);
	dw_mipi_dsi_phy_write(dsi, HS_RX_CONTROL_OF_LANE_0,
			      HSFREQRANGE_SEL(dppa_map[i].hsfreqrange));
	dw_mipi_dsi_phy_write(dsi, HS_RX_CONTROL_OF_LANE_1, 0);
	dw_mipi_dsi_phy_write(dsi, HS_RX_CONTROL_OF_LANE_2, 0);
	dw_mipi_dsi_phy_write(dsi, HS_RX_CONTROL_OF_LANE_3, 0);

	/* Normal operation */
	dw_mipi_dsi_phy_write(dsi, 0x0, 0);

	clk_disable_unprepare(dsi->phy_cfg_clk);
	clk_disable_unprepare(dsi->grf_clk);

	return ret;

err_pwr_on:
	clk_disable_unprepare(dsi->phy_cfg_clk);
err_phy_cfg_clk:
	clk_disable_unprepare(dsi->grf_clk);
err_grf_clk:
	clk_disable_unprepare(dsi->pclk);
err_pclk:
	pm_runtime_put(dsi->dev);
	return ret;
}

struct dw_mipi_dsi_dphy_power_on__HEXSHA { void _e3558747ebe1; };
struct dw_mipi_dsi_dphy_power_on__ATTRS { };
