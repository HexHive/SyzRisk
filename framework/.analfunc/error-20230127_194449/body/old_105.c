struct kfd_dev *kgd2kfd_probe(struct amdgpu_device *adev, bool vf)
{
	struct kfd_dev *kfd = NULL;
	const struct kfd2kgd_calls *f2g = NULL;
	struct pci_dev *pdev = adev->pdev;
	uint32_t gfx_target_version = 0;

	switch (adev->asic_type) {


	case CHIP_KAVERI:
		gfx_target_version = 70000;
		if (!vf)
			f2g = &gfx_v7_kfd2kgd;
		break;

	case CHIP_CARRIZO:
		gfx_target_version = 80001;
		if (!vf)
			f2g = &gfx_v8_kfd2kgd;
		break;


	case CHIP_HAWAII:
		gfx_target_version = 70001;
		if (!amdgpu_exp_hw_support)
			pr_info(
	"KFD support on Hawaii is experimental. See modparam exp_hw_support\n"
				);
		else if (!vf)
			f2g = &gfx_v7_kfd2kgd;
		break;

	case CHIP_TONGA:
		gfx_target_version = 80002;
		if (!vf)
			f2g = &gfx_v8_kfd2kgd;
		break;
	case CHIP_FIJI:
		gfx_target_version = 80003;
		f2g = &gfx_v8_kfd2kgd;
		break;
	case CHIP_POLARIS10:
		gfx_target_version = 80003;
		f2g = &gfx_v8_kfd2kgd;
		break;
	case CHIP_POLARIS11:
		gfx_target_version = 80003;
		if (!vf)
			f2g = &gfx_v8_kfd2kgd;
		break;
	case CHIP_POLARIS12:
		gfx_target_version = 80003;
		if (!vf)
			f2g = &gfx_v8_kfd2kgd;
		break;
	case CHIP_VEGAM:
		gfx_target_version = 80003;
		if (!vf)
			f2g = &gfx_v8_kfd2kgd;
		break;
	default:
		switch (adev->ip_versions[GC_HWIP][0]) {
		/* Vega 10 */
		case IP_VERSION(9, 0, 1):
			gfx_target_version = 90000;
			f2g = &gfx_v9_kfd2kgd;
			break;

		/* Raven */
		case IP_VERSION(9, 1, 0):
		case IP_VERSION(9, 2, 2):
			gfx_target_version = 90002;
			if (!vf)
				f2g = &gfx_v9_kfd2kgd;
			break;

		/* Vega12 */
		case IP_VERSION(9, 2, 1):
			gfx_target_version = 90004;
			if (!vf)
				f2g = &gfx_v9_kfd2kgd;
			break;
		/* Renoir */
		case IP_VERSION(9, 3, 0):
			gfx_target_version = 90012;
			if (!vf)
				f2g = &gfx_v9_kfd2kgd;
			break;
		/* Vega20 */
		case IP_VERSION(9, 4, 0):
			gfx_target_version = 90006;
			if (!vf)
				f2g = &gfx_v9_kfd2kgd;
			break;
		/* Arcturus */
		case IP_VERSION(9, 4, 1):
			gfx_target_version = 90008;
			f2g = &arcturus_kfd2kgd;
			break;
		/* Aldebaran */
		case IP_VERSION(9, 4, 2):
			gfx_target_version = 90010;
			f2g = &aldebaran_kfd2kgd;
			break;
		/* Navi10 */
		case IP_VERSION(10, 1, 10):
			gfx_target_version = 100100;
			if (!vf)
				f2g = &gfx_v10_kfd2kgd;
			break;
		/* Navi12 */
		case IP_VERSION(10, 1, 2):
			gfx_target_version = 100101;
			f2g = &gfx_v10_kfd2kgd;
			break;
		/* Navi14 */
		case IP_VERSION(10, 1, 1):
			gfx_target_version = 100102;
			if (!vf)
				f2g = &gfx_v10_kfd2kgd;
			break;
		/* Cyan Skillfish */
		case IP_VERSION(10, 1, 3):
		case IP_VERSION(10, 1, 4):
			gfx_target_version = 100103;
			if (!vf)
				f2g = &gfx_v10_kfd2kgd;
			break;
		/* Sienna Cichlid */
		case IP_VERSION(10, 3, 0):
			gfx_target_version = 100300;
			f2g = &gfx_v10_3_kfd2kgd;
			break;
		/* Navy Flounder */
		case IP_VERSION(10, 3, 2):
			gfx_target_version = 100301;
			f2g = &gfx_v10_3_kfd2kgd;
			break;
		/* Van Gogh */
		case IP_VERSION(10, 3, 1):
			gfx_target_version = 100303;
			if (!vf)
				f2g = &gfx_v10_3_kfd2kgd;
			break;
		/* Dimgrey Cavefish */
		case IP_VERSION(10, 3, 4):
			gfx_target_version = 100302;
			f2g = &gfx_v10_3_kfd2kgd;
			break;
		/* Beige Goby */
		case IP_VERSION(10, 3, 5):
			gfx_target_version = 100304;
			f2g = &gfx_v10_3_kfd2kgd;
			break;
		/* Yellow Carp */
		case IP_VERSION(10, 3, 3):
			gfx_target_version = 100305;
			if (!vf)
				f2g = &gfx_v10_3_kfd2kgd;
			break;
		case IP_VERSION(10, 3, 6):
		case IP_VERSION(10, 3, 7):
			gfx_target_version = 100306;
			if (!vf)
				f2g = &gfx_v10_3_kfd2kgd;
			break;
		case IP_VERSION(11, 0, 0):
			gfx_target_version = 110000;
			f2g = &gfx_v11_kfd2kgd;
			break;
		case IP_VERSION(11, 0, 1):
			gfx_target_version = 110003;
			f2g = &gfx_v11_kfd2kgd;
			break;
		case IP_VERSION(11, 0, 2):
			gfx_target_version = 110002;
			f2g = &gfx_v11_kfd2kgd;
			break;
		case IP_VERSION(11, 0, 3):
			/* Note: Compiler version is 11.0.1 while HW version is 11.0.3 */
			gfx_target_version = 110001;
			f2g = &gfx_v11_kfd2kgd;
			break;
		default:
			break;
		}
		break;
	}

	if (!f2g) {
		if (adev->ip_versions[GC_HWIP][0])
			dev_err(kfd_device, "GC IP %06x %s not supported in kfd\n",
				adev->ip_versions[GC_HWIP][0], vf ? "VF" : "");
		else
			dev_err(kfd_device, "%s %s not supported in kfd\n",
				amdgpu_asic_name[adev->asic_type], vf ? "VF" : "");
		return NULL;
	}

	kfd = kzalloc(sizeof(*kfd), GFP_KERNEL);
	if (!kfd)
		return NULL;

	kfd->adev = adev;
	kfd_device_info_init(kfd, vf, gfx_target_version);
	kfd->pdev = pdev;
	kfd->init_complete = false;
	kfd->kfd2kgd = f2g;
	atomic_set(&kfd->compute_profile, 0);

	mutex_init(&kfd->doorbell_mutex);
	memset(&kfd->doorbell_available_index, 0,
		sizeof(kfd->doorbell_available_index));

	atomic_set(&kfd->sram_ecc_flag, 0);

	ida_init(&kfd->doorbell_ida);

	return kfd;
}

struct kgd2kfd_probe__HEXSHA { void _d69a3b762dc4; };
struct kgd2kfd_probe__ATTRS { };
