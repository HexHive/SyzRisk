struct list_head *ghes_get_devices(void)
{
	int idx = -1;

	if (IS_ENABLED(CONFIG_X86)) {
		idx = acpi_match_platform_list(plat_list);
		if (idx < 0) {
			if (!ghes_edac_force_enable)
				return NULL;

			pr_warn_once("Force-loading ghes_edac on an unsupported platform. You're on your own!\n");
		}
	}

	return &ghes_devs;
}

struct ghes_get_devices__HEXSHA { void _9057a3f7ac36; };
struct ghes_get_devices__ATTRS { };
