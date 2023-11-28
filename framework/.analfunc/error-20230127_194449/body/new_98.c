static void rtw8852b_thermal_trim(struct rtw89_dev *rtwdev)
{
#define __thm_setting(raw)				\
({							\
	u8 __v = (raw);					\
	((__v & 0x1) << 3) | ((__v & 0x1f) >> 1);	\
})
	struct rtw89_power_trim_info *info = &rtwdev->pwr_trim;
	u8 i, val;

	if (!info->pg_thermal_trim) {
		rtw89_debug(rtwdev, RTW89_DBG_RFK,
			    "[THERMAL][TRIM] no PG, do nothing\n");

		return;
	}

	for (i = 0; i < RF_PATH_NUM_8852B; i++) {
		val = __thm_setting(info->thermal_trim[i]);
		rtw89_write_rf(rtwdev, i, RR_TM2, RR_TM2_OFF, val);

		rtw89_debug(rtwdev, RTW89_DBG_RFK,
			    "[THERMAL][TRIM] path=%d thermal_setting=0x%x\n",
			    i, val);
	}
#undef __thm_setting
}

struct rtw8852b_thermal_trim__HEXSHA { void _134cf7c01517; };
struct rtw8852b_thermal_trim__ATTRS { };
