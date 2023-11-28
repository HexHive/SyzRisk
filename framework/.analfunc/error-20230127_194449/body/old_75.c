static int stage2_apply_range(struct kvm *kvm, phys_addr_t addr,
			      phys_addr_t end,
			      int (*fn)(struct kvm_pgtable *, u64, u64),
			      bool resched)
{
	int ret;
	u64 next;

	do {
		struct kvm_pgtable *pgt = kvm->arch.mmu.pgt;
		if (!pgt)
			return -EINVAL;

		next = stage2_pgd_addr_end(kvm, addr, end);
		ret = fn(pgt, addr, next - addr);
		if (ret)
			break;

		if (resched && next != end)
			cond_resched_rwlock_write(&kvm->mmu_lock);
	} while (addr = next, addr != end);

	return ret;
}

struct stage2_apply_range__HEXSHA { void _5994bc9e05c2; };
struct stage2_apply_range__ATTRS { };
