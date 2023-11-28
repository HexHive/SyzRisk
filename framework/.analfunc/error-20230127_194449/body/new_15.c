static int qmp_pcie_power_on(struct phy *phy)
{
	struct qmp_phy *qphy = phy_get_drvdata(phy);
	struct qcom_qmp *qmp = qphy->qmp;
	const struct qmp_phy_cfg *cfg = qphy->cfg;
	const struct qmp_phy_cfg_tables *mode_tables;
	void __iomem *pcs = qphy->pcs;
	void __iomem *status;
	unsigned int mask, val;
	int ret;

	qphy_setbits(pcs, cfg->regs[QPHY_PCS_POWER_DOWN_CONTROL],
			cfg->pwrdn_ctrl);

	if (qphy->mode == PHY_MODE_PCIE_RC)
		mode_tables = cfg->tables_rc;
	else
		mode_tables = cfg->tables_ep;

	qmp_pcie_serdes_init(qphy, &cfg->tables);
	qmp_pcie_serdes_init(qphy, mode_tables);

	ret = clk_prepare_enable(qphy->pipe_clk);
	if (ret) {
		dev_err(qmp->dev, "pipe_clk enable failed err=%d\n", ret);
		return ret;
	}

	/* Tx, Rx, and PCS configurations */
	qmp_pcie_lanes_init(qphy, &cfg->tables);
	qmp_pcie_lanes_init(qphy, mode_tables);

	qmp_pcie_pcs_init(qphy, &cfg->tables);
	qmp_pcie_pcs_init(qphy, mode_tables);

	/* Pull PHY out of reset state */
	qphy_clrbits(pcs, cfg->regs[QPHY_SW_RESET], SW_RESET);

	/* start SerDes and Phy-Coding-Sublayer */
	qphy_setbits(pcs, cfg->regs[QPHY_START_CTRL], SERDES_START | PCS_START);

	if (!cfg->skip_start_delay)
		usleep_range(1000, 1200);

	status = pcs + cfg->regs[QPHY_PCS_STATUS];
	mask = cfg->phy_status;
	ret = readl_poll_timeout(status, val, !(val & mask), 200,
				 PHY_INIT_COMPLETE_TIMEOUT);
	if (ret) {
		dev_err(qmp->dev, "phy initialization timed-out\n");
		goto err_disable_pipe_clk;
	}

	return 0;

err_disable_pipe_clk:
	clk_disable_unprepare(qphy->pipe_clk);

	return ret;
}

struct qmp_pcie_power_on__HEXSHA { void _5806b87dea8f; };
struct qmp_pcie_power_on__ATTRS { };
