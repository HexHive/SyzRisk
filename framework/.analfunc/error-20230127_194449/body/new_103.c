static void rtw8852b_phycap_parsing_pa_bias_trim(struct rtw89_dev *rtwdev,
						 u8 *phycap_map)
{
	struct rtw89_power_trim_info *info = &rtwdev->pwr_trim;
	static const u32 pabias_trim_addr[RF_PATH_NUM_8852B] = {0x5DE, 0x5DB};
	u32 addr = rtwdev->chip->phycap_addr;
	u8 i;

	for (i = 0; i < RF_PATH_NUM_8852B; i++) {
		info->pa_bias_trim[i] = phycap_map[pabias_trim_addr[i] - addr];

		rtw89_debug(rtwdev, RTW89_DBG_RFK,
			    "[PA_BIAS][TRIM] path=%d pa_bias_trim=0x%x\n",
			    i, info->pa_bias_trim[i]);

		if (info->pa_bias_trim[i] != 0xff)
			info->pg_pa_bias_trim = true;
	}
}

struct rtw8852b_phycap_parsing_pa_bias_trim__HEXSHA { void _134cf7c01517; };
struct rtw8852b_phycap_parsing_pa_bias_trim__ATTRS { };
