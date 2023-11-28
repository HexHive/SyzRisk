static void rtw8852c_set_gain_error(struct rtw89_dev *rtwdev,
				    enum rtw89_subband subband,
				    enum rtw89_rf_path path)
{
	const struct rtw89_phy_bb_gain_info *gain = &rtwdev->bb_gain;
	u8 gain_band = rtw8852c_mapping_gain_band(subband);
	s32 val;
	u32 reg;
	u32 mask;
	int i;

	for (i = 0; i < LNA_GAIN_NUM; i++) {
		if (subband == RTW89_CH_2G)
			reg = bb_gain_lna[i].gain_g[path];
		else
			reg = bb_gain_lna[i].gain_a[path];

		mask = bb_gain_lna[i].gain_mask;
		val = gain->lna_gain[gain_band][path][i];
		rtw89_phy_write32_mask(rtwdev, reg, mask, val);

		if (subband == RTW89_CH_2G) {
			reg = bb_gain_bypass_lna[i].gain_g[path];
			mask = bb_gain_bypass_lna[i].gain_mask_g;
		} else {
			reg = bb_gain_bypass_lna[i].gain_a[path];
			mask = bb_gain_bypass_lna[i].gain_mask_a;
		}

		val = gain->lna_gain_bypass[gain_band][path][i];
		rtw89_phy_write32_mask(rtwdev, reg, mask, val);

		if (subband != RTW89_CH_2G) {
			reg = bb_gain_op1db_a.reg[i].lna[path];
			mask = bb_gain_op1db_a.reg[i].mask;
			val = gain->lna_op1db[gain_band][path][i];
			rtw89_phy_write32_mask(rtwdev, reg, mask, val);

			reg = bb_gain_op1db_a.reg[i].tia_lna[path];
			mask = bb_gain_op1db_a.reg[i].mask;
			val = gain->tia_lna_op1db[gain_band][path][i];
			rtw89_phy_write32_mask(rtwdev, reg, mask, val);
		}
	}

	if (subband != RTW89_CH_2G) {
		reg = bb_gain_op1db_a.reg_tia0_lna6[path];
		mask = bb_gain_op1db_a.mask_tia0_lna6;
		val = gain->tia_lna_op1db[gain_band][path][7];
		rtw89_phy_write32_mask(rtwdev, reg, mask, val);
	}

	for (i = 0; i < TIA_GAIN_NUM; i++) {
		if (subband == RTW89_CH_2G)
			reg = bb_gain_tia[i].gain_g[path];
		else
			reg = bb_gain_tia[i].gain_a[path];

		mask = bb_gain_tia[i].gain_mask;
		val = gain->tia_gain[gain_band][path][i];
		rtw89_phy_write32_mask(rtwdev, reg, mask, val);
	}
}

struct rtw8852c_set_gain_error__HEXSHA { void _6e5125bcbaf8; };
struct rtw8852c_set_gain_error__ATTRS { };
