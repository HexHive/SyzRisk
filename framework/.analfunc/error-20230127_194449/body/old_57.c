void ghes_edac_unregister(struct ghes *ghes)
{
	struct mem_ctl_info *mci;
	unsigned long flags;

	if (!force_load)
		return;

	mutex_lock(&ghes_reg_mutex);

	system_scanned = false;
	memset(&ghes_hw, 0, sizeof(struct ghes_hw_desc));

	if (!refcount_dec_and_test(&ghes_refcount))
		goto unlock;

	/*
	 * Wait for the irq handler being finished.
	 */
	spin_lock_irqsave(&ghes_lock, flags);
	mci = ghes_pvt ? ghes_pvt->mci : NULL;
	ghes_pvt = NULL;
	spin_unlock_irqrestore(&ghes_lock, flags);

	if (!mci)
		goto unlock;

	mci = edac_mc_del_mc(mci->pdev);
	if (mci)
		edac_mc_free(mci);

	ghes_unregister_report_chain(&ghes_edac_mem_err_nb);

unlock:
	mutex_unlock(&ghes_reg_mutex);
}

struct ghes_edac_unregister__HEXSHA { void _9057a3f7ac36; };
struct ghes_edac_unregister__ATTRS { };
