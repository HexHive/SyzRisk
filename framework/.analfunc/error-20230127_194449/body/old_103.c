static void kfd_find_numa_node_in_srat(struct kfd_dev *kdev)
{
	struct acpi_table_header *table_header = NULL;
	struct acpi_subtable_header *sub_header = NULL;
	unsigned long table_end, subtable_len;
	u32 pci_id = pci_domain_nr(kdev->pdev->bus) << 16 |
			pci_dev_id(kdev->pdev);
	u32 bdf;
	acpi_status status;
	struct acpi_srat_cpu_affinity *cpu;
	struct acpi_srat_generic_affinity *gpu;
	int pxm = 0, max_pxm = 0;
	int numa_node = NUMA_NO_NODE;
	bool found = false;

	/* Fetch the SRAT table from ACPI */
	status = acpi_get_table(ACPI_SIG_SRAT, 0, &table_header);
	if (status == AE_NOT_FOUND) {
		pr_warn("SRAT table not found\n");
		return;
	} else if (ACPI_FAILURE(status)) {
		const char *err = acpi_format_exception(status);
		pr_err("SRAT table error: %s\n", err);
		return;
	}

	table_end = (unsigned long)table_header + table_header->length;

	/* Parse all entries looking for a match. */
	sub_header = (struct acpi_subtable_header *)
			((unsigned long)table_header +
			sizeof(struct acpi_table_srat));
	subtable_len = sub_header->length;

	while (((unsigned long)sub_header) + subtable_len  < table_end) {
		/*
		 * If length is 0, break from this loop to avoid
		 * infinite loop.
		 */
		if (subtable_len == 0) {
			pr_err("SRAT invalid zero length\n");
			break;
		}

		switch (sub_header->type) {
		case ACPI_SRAT_TYPE_CPU_AFFINITY:
			cpu = (struct acpi_srat_cpu_affinity *)sub_header;
			pxm = *((u32 *)cpu->proximity_domain_hi) << 8 |
					cpu->proximity_domain_lo;
			if (pxm > max_pxm)
				max_pxm = pxm;
			break;
		case ACPI_SRAT_TYPE_GENERIC_AFFINITY:
			gpu = (struct acpi_srat_generic_affinity *)sub_header;
			bdf = *((u16 *)(&gpu->device_handle[0])) << 16 |
					*((u16 *)(&gpu->device_handle[2]));
			if (bdf == pci_id) {
				found = true;
				numa_node = pxm_to_node(gpu->proximity_domain);
			}
			break;
		default:
			break;
		}

		if (found)
			break;

		sub_header = (struct acpi_subtable_header *)
				((unsigned long)sub_header + subtable_len);
		subtable_len = sub_header->length;
	}

	acpi_put_table(table_header);

	/* Workaround bad cpu-gpu binding case */
	if (found && (numa_node < 0 ||
			numa_node > pxm_to_node(max_pxm)))
		numa_node = 0;

	if (numa_node != NUMA_NO_NODE)
		set_dev_node(&kdev->pdev->dev, numa_node);
}

struct kfd_find_numa_node_in_srat__HEXSHA { void _d69a3b762dc4; };
struct kfd_find_numa_node_in_srat__ATTRS { };
