static int kfd_fill_gpu_direct_io_link_to_cpu(int *avail_size,
			struct kfd_dev *kdev,
			struct crat_subtype_iolink *sub_type_hdr,
			uint32_t proximity_domain)
{
	*avail_size -= sizeof(struct crat_subtype_iolink);
	if (*avail_size < 0)
		return -ENOMEM;

	memset((void *)sub_type_hdr, 0, sizeof(struct crat_subtype_iolink));

	/* Fill in subtype header data */
	sub_type_hdr->type = CRAT_SUBTYPE_IOLINK_AFFINITY;
	sub_type_hdr->length = sizeof(struct crat_subtype_iolink);
	sub_type_hdr->flags |= CRAT_SUBTYPE_FLAGS_ENABLED;
	if (kfd_dev_is_large_bar(kdev))
		sub_type_hdr->flags |= CRAT_IOLINK_FLAGS_BI_DIRECTIONAL;

	/* Fill in IOLINK subtype.
	 * TODO: Fill-in other fields of iolink subtype
	 */
	if (kdev->adev->gmc.xgmi.connected_to_cpu) {
		/*
		 * with host gpu xgmi link, host can access gpu memory whether
		 * or not pcie bar type is large, so always create bidirectional
		 * io link.
		 */
		sub_type_hdr->flags |= CRAT_IOLINK_FLAGS_BI_DIRECTIONAL;
		sub_type_hdr->io_interface_type = CRAT_IOLINK_TYPE_XGMI;
		sub_type_hdr->num_hops_xgmi = 1;
		if (KFD_GC_VERSION(kdev) == IP_VERSION(9, 4, 2)) {
			sub_type_hdr->minimum_bandwidth_mbs =
					amdgpu_amdkfd_get_xgmi_bandwidth_mbytes(
							kdev->adev, NULL, true);
			sub_type_hdr->maximum_bandwidth_mbs =
					sub_type_hdr->minimum_bandwidth_mbs;
		}
	} else {
		sub_type_hdr->io_interface_type = CRAT_IOLINK_TYPE_PCIEXPRESS;
		sub_type_hdr->minimum_bandwidth_mbs =
				amdgpu_amdkfd_get_pcie_bandwidth_mbytes(kdev->adev, true);
		sub_type_hdr->maximum_bandwidth_mbs =
				amdgpu_amdkfd_get_pcie_bandwidth_mbytes(kdev->adev, false);
	}

	sub_type_hdr->proximity_domain_from = proximity_domain;


	if (kdev->pdev->dev.numa_node == NUMA_NO_NODE)
		kfd_find_numa_node_in_srat(kdev);


	if (kdev->pdev->dev.numa_node == NUMA_NO_NODE)
		sub_type_hdr->proximity_domain_to = 0;
	else
		sub_type_hdr->proximity_domain_to = kdev->pdev->dev.numa_node;



	return 0;
}

struct kfd_fill_gpu_direct_io_link_to_cpu__HEXSHA { void _d69a3b762dc4; };
struct kfd_fill_gpu_direct_io_link_to_cpu__ATTRS { };
