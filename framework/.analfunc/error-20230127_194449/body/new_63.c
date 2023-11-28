static void ipa_table_init_add(struct gsi_trans *trans, bool filter, bool ipv6)
{
	struct ipa *ipa = container_of(trans->gsi, struct ipa, gsi);
	const struct ipa_mem *hash_mem;
	enum ipa_cmd_opcode opcode;
	const struct ipa_mem *mem;
	dma_addr_t hash_addr;
	dma_addr_t addr;
	u32 zero_offset;
	u16 hash_count;
	u32 zero_size;
	u16 hash_size;
	u16 count;
	u16 size;

	opcode = filter ? ipv6 ? IPA_CMD_IP_V6_FILTER_INIT
			       : IPA_CMD_IP_V4_FILTER_INIT
			: ipv6 ? IPA_CMD_IP_V6_ROUTING_INIT
			       : IPA_CMD_IP_V4_ROUTING_INIT;

	mem = ipa_table_mem(ipa, filter, false, ipv6);
	hash_mem = ipa_table_mem(ipa, filter, true, ipv6);

	/* Compute the number of table entries to initialize */
	if (filter) {
		/* The number of filtering endpoints determines number of
		 * entries in the filter table; we also add one more "slot"
		 * to hold the bitmap itself.  The size of the hashed filter
		 * table is either the same as the non-hashed one, or zero.
		 */
		count = 1 + hweight32(ipa->filter_map);
		hash_count = hash_mem && hash_mem->size ? count : 0;
	} else {
		/* The size of a route table region determines the number
		 * of entries it has.
		 */
		count = mem->size / sizeof(__le64);
		hash_count = hash_mem && hash_mem->size / sizeof(__le64);
	}
	size = count * sizeof(__le64);
	hash_size = hash_count * sizeof(__le64);

	addr = ipa_table_addr(ipa, filter, count);
	hash_addr = ipa_table_addr(ipa, filter, hash_count);

	ipa_cmd_table_init_add(trans, opcode, size, mem->offset, addr,
			       hash_size, hash_mem->offset, hash_addr);
	if (!filter)
		return;

	/* Zero the unused space in the filter table */
	zero_offset = mem->offset + size;
	zero_size = mem->size - size;
	ipa_cmd_dma_shared_mem_add(trans, zero_offset, zero_size,
				   ipa->zero_addr, true);
	if (!hash_size)
		return;

	/* Zero the unused space in the hashed filter table */
	zero_offset = hash_mem->offset + hash_size;
	zero_size = hash_mem->size - hash_size;
	ipa_cmd_dma_shared_mem_add(trans, zero_offset, zero_size,
				   ipa->zero_addr, true);
}

struct ipa_table_init_add__HEXSHA { void _5cb76899fb47; };
struct ipa_table_init_add__ATTRS { };
