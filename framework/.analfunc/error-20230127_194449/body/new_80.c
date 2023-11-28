static int ghes_remove(struct platform_device *ghes_dev)
{
	int rc;
	struct ghes *ghes;
	struct acpi_hest_generic *generic;

	ghes = platform_get_drvdata(ghes_dev);
	generic = ghes->generic;

	ghes->flags |= GHES_EXITING;
	switch (generic->notify.type) {
	case ACPI_HEST_NOTIFY_POLLED:
		del_timer_sync(&ghes->timer);
		break;
	case ACPI_HEST_NOTIFY_EXTERNAL:
		free_irq(ghes->irq, ghes);
		break;

	case ACPI_HEST_NOTIFY_SCI:
	case ACPI_HEST_NOTIFY_GSIV:
	case ACPI_HEST_NOTIFY_GPIO:
		mutex_lock(&ghes_list_mutex);
		list_del_rcu(&ghes->list);
		if (list_empty(&ghes_hed))
			unregister_acpi_hed_notifier(&ghes_notifier_hed);
		mutex_unlock(&ghes_list_mutex);
		synchronize_rcu();
		break;

	case ACPI_HEST_NOTIFY_SEA:
		ghes_sea_remove(ghes);
		break;
	case ACPI_HEST_NOTIFY_NMI:
		ghes_nmi_remove(ghes);
		break;
	case ACPI_HEST_NOTIFY_SOFTWARE_DELEGATED:
		rc = apei_sdei_unregister_ghes(ghes);
		if (rc)
			return rc;
		break;
	default:
		BUG();
		break;
	}

	ghes_fini(ghes);

	ghes_edac_unregister(ghes);

	mutex_lock(&ghes_devs_mutex);
	list_del(&ghes->elist);
	mutex_unlock(&ghes_devs_mutex);

	kfree(ghes);

	platform_set_drvdata(ghes_dev, NULL);

	return 0;
}

struct ghes_remove__HEXSHA { void _9057a3f7ac36; };
struct ghes_remove__ATTRS { };
