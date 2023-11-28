static void rtw8852b_power_trim(struct rtw89_dev *rtwdev)
{
	rtw8852b_thermal_trim(rtwdev);
	rtw8852b_pa_bias_trim(rtwdev);
}

struct rtw8852b_power_trim__HEXSHA { void _134cf7c01517; };
struct rtw8852b_power_trim__ATTRS { };
