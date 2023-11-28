int ipa_table_setup(struct ipa *ipa)
{
	struct gsi_trans *trans;

	/* We will need at most 8 TREs:
	 * - IPv4:
	 *     - One for route table initialization (non-hashed and hashed)
	 *     - One for filter table initialization (non-hashed and hashed)
	 *     - One to zero unused entries in the non-hashed filter table
	 *     - One to zero unused entries in the hashed filter table
	 * - IPv6:
	 *     - One for route table initialization (non-hashed and hashed)
	 *     - One for filter table initialization (non-hashed and hashed)
	 *     - One to zero unused entries in the non-hashed filter table
	 *     - One to zero unused entries in the hashed filter table
	 * All platforms support at least 8 TREs in a transaction.
	 */
	trans = ipa_cmd_trans_alloc(ipa, 8);
	if (!trans) {
		dev_err(&ipa->pdev->dev, "no transaction for table setup\n");
		return -EBUSY;
	}

	ipa_table_init_add(trans, false, IPA_CMD_IP_V4_ROUTING_INIT,
			   IPA_MEM_V4_ROUTE, IPA_MEM_V4_ROUTE_HASHED);

	ipa_table_init_add(trans, false, IPA_CMD_IP_V6_ROUTING_INIT,
			   IPA_MEM_V6_ROUTE, IPA_MEM_V6_ROUTE_HASHED);

	ipa_table_init_add(trans, true, IPA_CMD_IP_V4_FILTER_INIT,
			   IPA_MEM_V4_FILTER, IPA_MEM_V4_FILTER_HASHED);

	ipa_table_init_add(trans, true, IPA_CMD_IP_V6_FILTER_INIT,
			   IPA_MEM_V6_FILTER, IPA_MEM_V6_FILTER_HASHED);

	gsi_trans_commit_wait(trans);

	return 0;
}

struct ipa_table_setup__HEXSHA { void _5cb76899fb47; };
struct ipa_table_setup__ATTRS { };
