static phys_addr_t stage2_range_addr_end(phys_addr_t addr, phys_addr_t end)
{
	phys_addr_t size = kvm_granule_size(KVM_PGTABLE_MIN_BLOCK_LEVEL);
	phys_addr_t boundary = ALIGN_DOWN(addr + size, size);

	return (boundary - 1 < end - 1) ? boundary : end;
}

struct stage2_range_addr_end__HEXSHA { void _5994bc9e05c2; };
struct stage2_range_addr_end__ATTRS { };
