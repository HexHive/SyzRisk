static void rtw8852b_phycap_parsing_tssi(struct rtw89_dev *rtwdev, u8 *phycap_map)
{
	struct rtw89_tssi_info *tssi = &rtwdev->tssi;
	static const u32 tssi_trim_addr[RF_PATH_NUM_8852B] = {0x5D6, 0x5AB};
	u32 addr = rtwdev->chip->phycap_addr;
	bool pg = false;
	u32 ofst;
	u8 i, j;

	for (i = 0; i < RF_PATH_NUM_8852B; i++) {
		for (j = 0; j < TSSI_TRIM_CH_GROUP_NUM; j++) {
			/* addrs are in decreasing order */
			ofst = tssi_trim_addr[i] - addr - j;
			tssi->tssi_trim[i][j] = phycap_map[ofst];

			if (phycap_map[ofst] != 0xff)
				pg = true;
		}
	}

	if (!pg) {
		memset(tssi->tssi_trim, 0, sizeof(tssi->tssi_trim));
		rtw89_debug(rtwdev, RTW89_DBG_TSSI,
			    "[TSSI][TRIM] no PG, set all trim info to 0\n");
	}

	for (i = 0; i < RF_PATH_NUM_8852B; i++)
		for (j = 0; j < TSSI_TRIM_CH_GROUP_NUM; j++)
			rtw89_debug(rtwdev, RTW89_DBG_TSSI,
				    "[TSSI] path=%d idx=%d trim=0x%x addr=0x%x\n",
				    i, j, tssi->tssi_trim[i][j],
				    tssi_trim_addr[i] - j);
}

struct rtw8852b_phycap_parsing_tssi__HEXSHA { void _134cf7c01517; };
struct rtw8852b_phycap_parsing_tssi__ATTRS { };
