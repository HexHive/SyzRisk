static void rtw8852b_phycap_parsing_gain_comp(struct rtw89_dev *rtwdev, u8 *phycap_map)
{
	static const u32 comp_addrs[][RTW89_SUBBAND_2GHZ_5GHZ_NR] = {
		{0x5BB, 0x5BA, 0, 0x5B9, 0x5B8},
		{0x590, 0x58F, 0, 0x58E, 0x58D},
	};
	struct rtw89_phy_efuse_gain *gain = &rtwdev->efuse_gain;
	u32 phycap_addr = rtwdev->chip->phycap_addr;
	bool valid = false;
	int path, i;
	u8 data;

	for (path = 0; path < 2; path++)
		for (i = 0; i < RTW89_SUBBAND_2GHZ_5GHZ_NR; i++) {
			if (comp_addrs[path][i] == 0)
				continue;

			data = phycap_map[comp_addrs[path][i] - phycap_addr];
			valid |= _decode_efuse_gain(data, NULL,
						    &gain->comp[path][i]);
		}

	gain->comp_valid = valid;
}

struct rtw8852b_phycap_parsing_gain_comp__HEXSHA { void _134cf7c01517; };
struct rtw8852b_phycap_parsing_gain_comp__ATTRS { };
