static inline phys_addr_t
stage2_pgd_addr_end(struct kvm *kvm, phys_addr_t addr, phys_addr_t end)
{
	phys_addr_t boundary = (addr + stage2_pgdir_size(kvm)) & stage2_pgdir_mask(kvm);

	return (boundary - 1 < end - 1) ? boundary : end;
}

struct stage2_pgd_addr_end__HEXSHA { void _5994bc9e05c2; };
struct stage2_pgd_addr_end__ATTRS { };
