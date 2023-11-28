static enum rtw89_phy_bb_gain_band
rtw8852c_mapping_gain_band(enum rtw89_subband subband)
{
	switch (subband) {
	default:
	case RTW89_CH_2G:
		return RTW89_BB_GAIN_BAND_2G;
	case RTW89_CH_5G_BAND_1:
		return RTW89_BB_GAIN_BAND_5G_L;
	case RTW89_CH_5G_BAND_3:
		return RTW89_BB_GAIN_BAND_5G_M;
	case RTW89_CH_5G_BAND_4:
		return RTW89_BB_GAIN_BAND_5G_H;
	case RTW89_CH_6G_BAND_IDX0:
	case RTW89_CH_6G_BAND_IDX1:
		return RTW89_BB_GAIN_BAND_6G_L;
	case RTW89_CH_6G_BAND_IDX2:
	case RTW89_CH_6G_BAND_IDX3:
		return RTW89_BB_GAIN_BAND_6G_M;
	case RTW89_CH_6G_BAND_IDX4:
	case RTW89_CH_6G_BAND_IDX5:
		return RTW89_BB_GAIN_BAND_6G_H;
	case RTW89_CH_6G_BAND_IDX6:
	case RTW89_CH_6G_BAND_IDX7:
		return RTW89_BB_GAIN_BAND_6G_UH;
	}
}

struct rtw8852c_mapping_gain_band__HEXSHA { void _6e5125bcbaf8; };
struct rtw8852c_mapping_gain_band__ATTRS { };
