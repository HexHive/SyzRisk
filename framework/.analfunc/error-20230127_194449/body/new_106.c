static void rtw8852b_phycap_parsing_power_cal(struct rtw89_dev *rtwdev, u8 *phycap_map)
{
#define PWR_K_CHK_OFFSET 0x5E9
#define PWR_K_CHK_VALUE 0xAA
	u32 offset = PWR_K_CHK_OFFSET - rtwdev->chip->phycap_addr;

	if (phycap_map[offset] == PWR_K_CHK_VALUE)
		rtwdev->efuse.power_k_valid = true;
}

struct rtw8852b_phycap_parsing_power_cal__HEXSHA { void _134cf7c01517; };
struct rtw8852b_phycap_parsing_power_cal__ATTRS { };
