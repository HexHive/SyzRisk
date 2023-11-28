int pci_enable_pasid(struct pci_dev *pdev, int features)
{
	u16 control, supported;
	int pasid = pdev->pasid_cap;

	/*
	 * VFs must not implement the PASID Capability, but if a PF
	 * supports PASID, its VFs share the PF PASID configuration.
	 */
	if (pdev->is_virtfn) {
		if (pci_physfn(pdev)->pasid_enabled)
			return 0;
		return -EINVAL;
	}

	if (WARN_ON(pdev->pasid_enabled))
		return -EBUSY;

	if (!pdev->eetlp_prefix_path && !pdev->pasid_no_tlp)
		return -EINVAL;

	if (!pasid)
		return -EINVAL;

	if (!pci_acs_path_enabled(pdev, NULL, PCI_ACS_RR | PCI_ACS_UF))
		return -EINVAL;

	pci_read_config_word(pdev, pasid + PCI_PASID_CAP, &supported);
	supported &= PCI_PASID_CAP_EXEC | PCI_PASID_CAP_PRIV;

	/* User wants to enable anything unsupported? */
	if ((supported & features) != features)
		return -EINVAL;

	control = PCI_PASID_CTRL_ENABLE | features;
	pdev->pasid_features = features;

	pci_write_config_word(pdev, pasid + PCI_PASID_CTRL, control);

	pdev->pasid_enabled = 1;

	return 0;
}

struct pci_enable_pasid__HEXSHA { void _201007ef707a; };
struct pci_enable_pasid__ATTRS { };
