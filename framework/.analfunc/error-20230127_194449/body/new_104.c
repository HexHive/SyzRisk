static void rtw8852b_phycap_parsing_thermal_trim(struct rtw89_dev *rtwdev,
						 u8 *phycap_map)
{
	struct rtw89_power_trim_info *info = &rtwdev->pwr_trim;
	static const u32 thm_trim_addr[RF_PATH_NUM_8852B] = {0x5DF, 0x5DC};
	u32 addr = rtwdev->chip->phycap_addr;
	u8 i;

	for (i = 0; i < RF_PATH_NUM_8852B; i++) {
		info->thermal_trim[i] = phycap_map[thm_trim_addr[i] - addr];

		rtw89_debug(rtwdev, RTW89_DBG_RFK,
			    "[THERMAL][TRIM] path=%d thermal_trim=0x%x\n",
			    i, info->thermal_trim[i]);

		if (info->thermal_trim[i] != 0xff)
			info->pg_thermal_trim = true;
	}
}

struct rtw8852b_phycap_parsing_thermal_trim__HEXSHA { void _134cf7c01517; };
struct rtw8852b_phycap_parsing_thermal_trim__ATTRS { };
