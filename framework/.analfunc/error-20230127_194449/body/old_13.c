static int qmp_pcie_power_off(struct phy *phy)
{
	struct qmp_phy *qphy = phy_get_drvdata(phy);
	const struct qmp_phy_cfg *cfg = qphy->cfg;

	clk_disable_unprepare(qphy->pipe_clk);

	/* PHY reset */
	qphy_setbits(qphy->pcs, cfg->regs[QPHY_SW_RESET], SW_RESET);

	/* stop SerDes and Phy-Coding-Sublayer */
	qphy_clrbits(qphy->pcs, cfg->regs[QPHY_START_CTRL], cfg->start_ctrl);

	/* Put PHY into POWER DOWN state: active low */
	qphy_clrbits(qphy->pcs, cfg->regs[QPHY_PCS_POWER_DOWN_CONTROL],
			cfg->pwrdn_ctrl);

	return 0;
}

struct qmp_pcie_power_off__HEXSHA { void _5806b87dea8f; };
struct qmp_pcie_power_off__ATTRS { };
