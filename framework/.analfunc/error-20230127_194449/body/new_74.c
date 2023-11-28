enum rtw89_gain_offset rtw89_subband_to_gain_offset_band_of_ofdm(enum rtw89_subband subband)
{
	switch (subband) {
	default:
	case RTW89_CH_2G:
		return RTW89_GAIN_OFFSET_2G_OFDM;
	case RTW89_CH_5G_BAND_1:
		return RTW89_GAIN_OFFSET_5G_LOW;
	case RTW89_CH_5G_BAND_3:
		return RTW89_GAIN_OFFSET_5G_MID;
	case RTW89_CH_5G_BAND_4:
		return RTW89_GAIN_OFFSET_5G_HIGH;
	}
}

struct rtw89_subband_to_gain_offset_band_of_ofdm__HEXSHA { void _6e5125bcbaf8; };
struct rtw89_subband_to_gain_offset_band_of_ofdm__ATTRS { };
