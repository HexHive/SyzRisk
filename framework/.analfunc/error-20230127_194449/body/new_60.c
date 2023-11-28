static int
rtw89_debug_priv_mac_mem_dump_get(struct seq_file *m, void *v)
{
	struct rtw89_debugfs_priv *debugfs_priv = m->private;
	struct rtw89_dev *rtwdev = debugfs_priv->rtwdev;
	bool grant_read = false;

	if (debugfs_priv->mac_mem.sel >= RTW89_MAC_MEM_NUM)
		return -ENOENT;

	if (rtwdev->chip->chip_id == RTL8852C) {
		switch (debugfs_priv->mac_mem.sel) {
		case RTW89_MAC_MEM_TXD_FIFO_0_V1:
		case RTW89_MAC_MEM_TXD_FIFO_1_V1:
		case RTW89_MAC_MEM_TXDATA_FIFO_0:
		case RTW89_MAC_MEM_TXDATA_FIFO_1:
			grant_read = true;
			break;
		default:
			break;
		}
	}

	mutex_lock(&rtwdev->mutex);
	rtw89_leave_ps_mode(rtwdev);
	if (grant_read)
		rtw89_write32_set(rtwdev, R_AX_TCR1, B_AX_TCR_FORCE_READ_TXDFIFO);
	rtw89_debug_dump_mac_mem(m, rtwdev,
				 debugfs_priv->mac_mem.sel,
				 debugfs_priv->mac_mem.start,
				 debugfs_priv->mac_mem.len);
	if (grant_read)
		rtw89_write32_clr(rtwdev, R_AX_TCR1, B_AX_TCR_FORCE_READ_TXDFIFO);
	mutex_unlock(&rtwdev->mutex);

	return 0;
}

struct rtw89_debug_priv_mac_mem_dump_get__HEXSHA { void _732dd91db3d3; };
struct rtw89_debug_priv_mac_mem_dump_get__ATTRS { };
