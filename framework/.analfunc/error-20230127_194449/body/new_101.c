static int rtw8852b_read_phycap(struct rtw89_dev *rtwdev, u8 *phycap_map)
{
	rtw8852b_phycap_parsing_power_cal(rtwdev, phycap_map);
	rtw8852b_phycap_parsing_tssi(rtwdev, phycap_map);
	rtw8852b_phycap_parsing_thermal_trim(rtwdev, phycap_map);
	rtw8852b_phycap_parsing_pa_bias_trim(rtwdev, phycap_map);
	rtw8852b_phycap_parsing_gain_comp(rtwdev, phycap_map);

	return 0;
}

struct rtw8852b_read_phycap__HEXSHA { void _134cf7c01517; };
struct rtw8852b_read_phycap__ATTRS { };
