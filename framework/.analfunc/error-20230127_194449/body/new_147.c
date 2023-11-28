static int mlx5_pci_link_toggle(struct mlx5_core_dev *dev)
{
	struct pci_bus *bridge_bus = dev->pdev->bus;
	struct pci_dev *bridge = bridge_bus->self;
	u16 reg16, dev_id, sdev_id;
	unsigned long timeout;
	struct pci_dev *sdev;
	int cap, err;
	u32 reg32;

	/* Check that all functions under the pci bridge are PFs of
	 * this device otherwise fail this function.
	 */
	err = pci_read_config_word(dev->pdev, PCI_DEVICE_ID, &dev_id);
	if (err)
		return err;
	list_for_each_entry(sdev, &bridge_bus->devices, bus_list) {
		err = pci_read_config_word(sdev, PCI_DEVICE_ID, &sdev_id);
		if (err)
			return err;
		if (sdev_id != dev_id)
			return -EPERM;
	}

	cap = pci_find_capability(bridge, PCI_CAP_ID_EXP);
	if (!cap)
		return -EOPNOTSUPP;

	list_for_each_entry(sdev, &bridge_bus->devices, bus_list) {
		pci_save_state(sdev);
		pci_cfg_access_lock(sdev);
	}
	/* PCI link toggle */
	err = pci_read_config_word(bridge, cap + PCI_EXP_LNKCTL, &reg16);
	if (err)
		return err;
	reg16 |= PCI_EXP_LNKCTL_LD;
	err = pci_write_config_word(bridge, cap + PCI_EXP_LNKCTL, reg16);
	if (err)
		return err;
	msleep(500);
	reg16 &= ~PCI_EXP_LNKCTL_LD;
	err = pci_write_config_word(bridge, cap + PCI_EXP_LNKCTL, reg16);
	if (err)
		return err;

	/* Check link */
	err = pci_read_config_dword(bridge, cap + PCI_EXP_LNKCAP, &reg32);
	if (err)
		return err;
	if (!(reg32 & PCI_EXP_LNKCAP_DLLLARC)) {
		mlx5_core_warn(dev, "No PCI link reporting capability (0x%08x)\n", reg32);
		msleep(1000);
		goto restore;
	}

	timeout = jiffies + msecs_to_jiffies(mlx5_tout_ms(dev, PCI_TOGGLE));
	do {
		err = pci_read_config_word(bridge, cap + PCI_EXP_LNKSTA, &reg16);
		if (err)
			return err;
		if (reg16 & PCI_EXP_LNKSTA_DLLLA)
			break;
		msleep(20);
	} while (!time_after(jiffies, timeout));

	if (reg16 & PCI_EXP_LNKSTA_DLLLA) {
		mlx5_core_info(dev, "PCI Link up\n");
	} else {
		mlx5_core_err(dev, "PCI link not ready (0x%04x) after %llu ms\n",
			      reg16, mlx5_tout_ms(dev, PCI_TOGGLE));
		err = -ETIMEDOUT;
	}

	do {
		err = pci_read_config_word(dev->pdev, PCI_DEVICE_ID, &reg16);
		if (err)
			return err;
		if (reg16 == dev_id)
			break;
		msleep(20);
	} while (!time_after(jiffies, timeout));

	if (reg16 == dev_id) {
		mlx5_core_info(dev, "Firmware responds to PCI config cycles again\n");
	} else {
		mlx5_core_err(dev, "Firmware is not responsive (0x%04x) after %llu ms\n",
			      reg16, mlx5_tout_ms(dev, PCI_TOGGLE));
		err = -ETIMEDOUT;
	}

restore:
	list_for_each_entry(sdev, &bridge_bus->devices, bus_list) {
		pci_cfg_access_unlock(sdev);
		pci_restore_state(sdev);
	}

	return err;
}

struct mlx5_pci_link_toggle__HEXSHA { void _212b4d7251c1; };
struct mlx5_pci_link_toggle__ATTRS { };
