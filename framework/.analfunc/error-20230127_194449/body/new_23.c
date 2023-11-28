static int rtw8852c_pwr_on_func(struct rtw89_dev *rtwdev)
{
	u32 val32;
	u32 ret;

	val32 = rtw89_read32_mask(rtwdev, R_AX_SYS_STATUS1, B_AX_PAD_HCI_SEL_V2_MASK);
	if (val32 == MAC_AX_HCI_SEL_PCIE_USB)
		rtw89_write32_set(rtwdev, R_AX_LDO_AON_CTRL0, B_AX_PD_REGU_L);

	rtw89_write32_clr(rtwdev, R_AX_SYS_PW_CTRL, B_AX_AFSM_WLSUS_EN |
						    B_AX_AFSM_PCIE_SUS_EN);
	rtw89_write32_set(rtwdev, R_AX_SYS_PW_CTRL, B_AX_DIS_WLBT_PDNSUSEN_SOPC);
	rtw89_write32_set(rtwdev, R_AX_WLLPS_CTRL, B_AX_DIS_WLBT_LPSEN_LOPC);
	rtw89_write32_clr(rtwdev, R_AX_SYS_PW_CTRL, B_AX_APDM_HPDN);
	rtw89_write32_clr(rtwdev, R_AX_SYS_PW_CTRL, B_AX_APFM_SWLPS);

	ret = read_poll_timeout(rtw89_read32, val32, val32 & B_AX_RDY_SYSPWR,
				1000, 20000, false, rtwdev, R_AX_SYS_PW_CTRL);
	if (ret)
		return ret;

	rtw89_write32_set(rtwdev, R_AX_SYS_PW_CTRL, B_AX_EN_WLON);
	rtw89_write32_set(rtwdev, R_AX_SYS_PW_CTRL, B_AX_APFN_ONMAC);

	ret = read_poll_timeout(rtw89_read32, val32, !(val32 & B_AX_APFN_ONMAC),
				1000, 20000, false, rtwdev, R_AX_SYS_PW_CTRL);
	if (ret)
		return ret;

	rtw89_write8_set(rtwdev, R_AX_PLATFORM_ENABLE, B_AX_PLATFORM_EN);
	rtw89_write8_clr(rtwdev, R_AX_PLATFORM_ENABLE, B_AX_PLATFORM_EN);
	rtw89_write8_set(rtwdev, R_AX_PLATFORM_ENABLE, B_AX_PLATFORM_EN);
	rtw89_write8_clr(rtwdev, R_AX_PLATFORM_ENABLE, B_AX_PLATFORM_EN);

	rtw89_write8_set(rtwdev, R_AX_PLATFORM_ENABLE, B_AX_PLATFORM_EN);
	rtw89_write32_clr(rtwdev, R_AX_SYS_SDIO_CTRL, B_AX_PCIE_CALIB_EN_V1);

	rtw89_write32_clr(rtwdev, R_AX_SYS_ISO_CTRL_EXTEND, B_AX_CMAC1_FEN);
	rtw89_write32_set(rtwdev, R_AX_SYS_ISO_CTRL_EXTEND, B_AX_R_SYM_ISO_CMAC12PP);
	rtw89_write32_clr(rtwdev, R_AX_AFE_CTRL1, B_AX_R_SYM_WLCMAC1_P4_PC_EN |
						  B_AX_R_SYM_WLCMAC1_P3_PC_EN |
						  B_AX_R_SYM_WLCMAC1_P2_PC_EN |
						  B_AX_R_SYM_WLCMAC1_P1_PC_EN |
						  B_AX_R_SYM_WLCMAC1_PC_EN);
	rtw89_write32_set(rtwdev, R_AX_SYS_ADIE_PAD_PWR_CTRL, B_AX_SYM_PADPDN_WL_PTA_1P3);

	ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL,
				      XTAL_SI_GND_SHDN_WL, XTAL_SI_GND_SHDN_WL);
	if (ret)
		return ret;

	rtw89_write32_set(rtwdev, R_AX_SYS_ADIE_PAD_PWR_CTRL, B_AX_SYM_PADPDN_WL_RFC_1P3);

	ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL,
				      XTAL_SI_SHDN_WL, XTAL_SI_SHDN_WL);
	if (ret)
		return ret;
	ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL, XTAL_SI_OFF_WEI,
				      XTAL_SI_OFF_WEI);
	if (ret)
		return ret;
	ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL, XTAL_SI_OFF_EI,
				      XTAL_SI_OFF_EI);
	if (ret)
		return ret;
	ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL, 0, XTAL_SI_RFC2RF);
	if (ret)
		return ret;
	ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL, XTAL_SI_PON_WEI,
				      XTAL_SI_PON_WEI);
	if (ret)
		return ret;
	ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL, XTAL_SI_PON_EI,
				      XTAL_SI_PON_EI);
	if (ret)
		return ret;
	ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_ANAPAR_WL, 0, XTAL_SI_SRAM2RFC);
	if (ret)
		return ret;
	ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_XTAL_XMD_2, 0, XTAL_SI_LDO_LPS);
	if (ret)
		return ret;
	ret = rtw89_mac_write_xtal_si(rtwdev, XTAL_SI_XTAL_XMD_4, 0, XTAL_SI_LPS_CAP);
	if (ret)
		return ret;

	rtw89_write32_set(rtwdev, R_AX_PMC_DBG_CTRL2, B_AX_SYSON_DIS_PMCR_AX_WRMSK);
	rtw89_write32_set(rtwdev, R_AX_SYS_ISO_CTRL, B_AX_ISO_EB2CORE);
	rtw89_write32_clr(rtwdev, R_AX_SYS_ISO_CTRL, B_AX_PWC_EV2EF_B15);

	fsleep(1000);

	rtw89_write32_clr(rtwdev, R_AX_SYS_ISO_CTRL, B_AX_PWC_EV2EF_B14);
	rtw89_write32_clr(rtwdev, R_AX_PMC_DBG_CTRL2, B_AX_SYSON_DIS_PMCR_AX_WRMSK);
	rtw89_write32_set(rtwdev, R_AX_GPIO0_15_EECS_EESK_LED1_PULL_LOW_EN,
			  B_AX_EECS_PULL_LOW_EN | B_AX_EESK_PULL_LOW_EN |
			  B_AX_LED1_PULL_LOW_EN);

	rtw89_write32_set(rtwdev, R_AX_DMAC_FUNC_EN,
			  B_AX_MAC_FUNC_EN | B_AX_DMAC_FUNC_EN | B_AX_MPDU_PROC_EN |
			  B_AX_WD_RLS_EN | B_AX_DLE_WDE_EN | B_AX_TXPKT_CTRL_EN |
			  B_AX_STA_SCH_EN | B_AX_DLE_PLE_EN | B_AX_PKT_BUF_EN |
			  B_AX_DMAC_TBL_EN | B_AX_PKT_IN_EN | B_AX_DLE_CPUIO_EN |
			  B_AX_DISPATCHER_EN | B_AX_BBRPT_EN | B_AX_MAC_SEC_EN |
			  B_AX_MAC_UN_EN | B_AX_H_AXIDMA_EN);

	rtw89_write32_set(rtwdev, R_AX_CMAC_FUNC_EN,
			  B_AX_CMAC_EN | B_AX_CMAC_TXEN | B_AX_CMAC_RXEN |
			  B_AX_FORCE_CMACREG_GCKEN | B_AX_PHYINTF_EN |
			  B_AX_CMAC_DMA_EN | B_AX_PTCLTOP_EN | B_AX_SCHEDULER_EN |
			  B_AX_TMAC_EN | B_AX_RMAC_EN);

	rtw89_write32_mask(rtwdev, R_AX_LED1_FUNC_SEL, B_AX_PINMUX_EESK_FUNC_SEL_V1_MASK,
			   PINMUX_EESK_FUNC_SEL_BT_LOG);

	return 0;
}

struct rtw8852c_pwr_on_func__HEXSHA { void _d187691ab63f; };
struct rtw8852c_pwr_on_func__ATTRS { };
