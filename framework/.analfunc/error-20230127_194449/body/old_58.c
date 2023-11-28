int ghes_edac_register(struct ghes *ghes, struct device *dev)
{
	bool fake = false;
	struct mem_ctl_info *mci;
	struct ghes_pvt *pvt;
	struct edac_mc_layer layers[1];
	unsigned long flags;
	int idx = -1;
	int rc = 0;

	if (IS_ENABLED(CONFIG_X86)) {
		/* Check if safe to enable on this system */
		idx = acpi_match_platform_list(plat_list);
		if (!force_load && idx < 0)
			return -ENODEV;
	} else {
		force_load = true;
		idx = 0;
	}

	/* finish another registration/unregistration instance first */
	mutex_lock(&ghes_reg_mutex);

	/*
	 * We have only one logical memory controller to which all DIMMs belong.
	 */
	if (refcount_inc_not_zero(&ghes_refcount))
		goto unlock;

	ghes_scan_system();

	/* Check if we've got a bogus BIOS */
	if (!ghes_hw.num_dimms) {
		fake = true;
		ghes_hw.num_dimms = 1;
	}

	layers[0].type = EDAC_MC_LAYER_ALL_MEM;
	layers[0].size = ghes_hw.num_dimms;
	layers[0].is_virt_csrow = true;

	mci = edac_mc_alloc(0, ARRAY_SIZE(layers), layers, sizeof(struct ghes_pvt));
	if (!mci) {
		pr_info("Can't allocate memory for EDAC data\n");
		rc = -ENOMEM;
		goto unlock;
	}

	pvt		= mci->pvt_info;
	pvt->mci	= mci;

	mci->pdev = dev;
	mci->mtype_cap = MEM_FLAG_EMPTY;
	mci->edac_ctl_cap = EDAC_FLAG_NONE;
	mci->edac_cap = EDAC_FLAG_NONE;
	mci->mod_name = "ghes_edac.c";
	mci->ctl_name = "ghes_edac";
	mci->dev_name = "ghes";

	if (fake) {
		pr_info("This system has a very crappy BIOS: It doesn't even list the DIMMS.\n");
		pr_info("Its SMBIOS info is wrong. It is doubtful that the error report would\n");
		pr_info("work on such system. Use this driver with caution\n");
	} else if (idx < 0) {
		pr_info("This EDAC driver relies on BIOS to enumerate memory and get error reports.\n");
		pr_info("Unfortunately, not all BIOSes reflect the memory layout correctly.\n");
		pr_info("So, the end result of using this driver varies from vendor to vendor.\n");
		pr_info("If you find incorrect reports, please contact your hardware vendor\n");
		pr_info("to correct its BIOS.\n");
		pr_info("This system has %d DIMM sockets.\n", ghes_hw.num_dimms);
	}

	if (!fake) {
		struct dimm_info *src, *dst;
		int i = 0;

		mci_for_each_dimm(mci, dst) {
			src = &ghes_hw.dimms[i];

			dst->idx	   = src->idx;
			dst->smbios_handle = src->smbios_handle;
			dst->nr_pages	   = src->nr_pages;
			dst->mtype	   = src->mtype;
			dst->edac_mode	   = src->edac_mode;
			dst->dtype	   = src->dtype;
			dst->grain	   = src->grain;

			/*
			 * If no src->label, preserve default label assigned
			 * from EDAC core.
			 */
			if (strlen(src->label))
				memcpy(dst->label, src->label, sizeof(src->label));

			i++;
		}

	} else {
		struct dimm_info *dimm = edac_get_dimm(mci, 0, 0, 0);

		dimm->nr_pages = 1;
		dimm->grain = 128;
		dimm->mtype = MEM_UNKNOWN;
		dimm->dtype = DEV_UNKNOWN;
		dimm->edac_mode = EDAC_SECDED;
	}

	rc = edac_mc_add_mc(mci);
	if (rc < 0) {
		pr_info("Can't register with the EDAC core\n");
		edac_mc_free(mci);
		rc = -ENODEV;
		goto unlock;
	}

	spin_lock_irqsave(&ghes_lock, flags);
	ghes_pvt = pvt;
	spin_unlock_irqrestore(&ghes_lock, flags);

	ghes_register_report_chain(&ghes_edac_mem_err_nb);

	/* only set on success */
	refcount_set(&ghes_refcount, 1);

unlock:

	/* Not needed anymore */
	kfree(ghes_hw.dimms);
	ghes_hw.dimms = NULL;

	mutex_unlock(&ghes_reg_mutex);

	return rc;
}

struct ghes_edac_register__HEXSHA { void _9057a3f7ac36; };
struct ghes_edac_register__ATTRS { };
