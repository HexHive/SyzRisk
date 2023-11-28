static void rtw8852c_set_gain_offset(struct rtw89_dev *rtwdev,
				     const struct rtw89_chan *chan,
				     enum rtw89_phy_idx phy_idx,
				     enum rtw89_rf_path path)
{
	static const u32 rssi_ofst_addr[2] = {R_PATH0_G_TIA0_LNA6_OP1DB_V1,
					      R_PATH1_G_TIA0_LNA6_OP1DB_V1};
	static const u32 rpl_mask[2] = {B_RPL_PATHA_MASK, B_RPL_PATHB_MASK};
	static const u32 rpl_tb_mask[2] = {B_RSSI_M_PATHA_MASK, B_RSSI_M_PATHB_MASK};
	struct rtw89_phy_efuse_gain *efuse_gain = &rtwdev->efuse_gain;
	enum rtw89_gain_offset gain_band;
	s32 offset_q0, offset_base_q4;
	s32 tmp = 0;

	if (!efuse_gain->offset_valid)
		return;

	if (rtwdev->dbcc_en && path == RF_PATH_B)
		phy_idx = RTW89_PHY_1;

	if (chan->band_type == RTW89_BAND_2G) {
		offset_q0 = efuse_gain->offset[path][RTW89_GAIN_OFFSET_2G_CCK];
		offset_base_q4 = efuse_gain->offset_base[phy_idx];

		tmp = clamp_t(s32, (-offset_q0 << 3) + (offset_base_q4 >> 1),
			      S8_MIN >> 1, S8_MAX >> 1);
		rtw89_phy_write32_mask(rtwdev, R_RPL_OFST, B_RPL_OFST_MASK, tmp & 0x7f);
	}

	switch (chan->subband_type) {
	default:
	case RTW89_CH_2G:
		gain_band = RTW89_GAIN_OFFSET_2G_OFDM;
		break;
	case RTW89_CH_5G_BAND_1:
		gain_band = RTW89_GAIN_OFFSET_5G_LOW;
		break;
	case RTW89_CH_5G_BAND_3:
		gain_band = RTW89_GAIN_OFFSET_5G_MID;
		break;
	case RTW89_CH_5G_BAND_4:
		gain_band = RTW89_GAIN_OFFSET_5G_HIGH;
		break;
	}

	offset_q0 = -efuse_gain->offset[path][gain_band];
	offset_base_q4 = efuse_gain->offset_base[phy_idx];

	tmp = (offset_q0 << 2) + (offset_base_q4 >> 2);
	tmp = clamp_t(s32, -tmp, S8_MIN, S8_MAX);
	rtw89_phy_write32_mask(rtwdev, rssi_ofst_addr[path], B_PATH0_R_G_OFST_MASK, tmp & 0xff);

	tmp = clamp_t(s32, offset_q0 << 4, S8_MIN, S8_MAX);
	rtw89_phy_write32_idx(rtwdev, R_RPL_PATHAB, rpl_mask[path], tmp & 0xff, phy_idx);
	rtw89_phy_write32_idx(rtwdev, R_RSSI_M_PATHAB, rpl_tb_mask[path], tmp & 0xff, phy_idx);
}

struct rtw8852c_set_gain_offset__HEXSHA { void _6e5125bcbaf8; };
struct rtw8852c_set_gain_offset__ATTRS { };
