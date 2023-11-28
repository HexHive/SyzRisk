long mem_cgroup_get_nr_swap_pages(struct mem_cgroup *memcg)
{
	long nr_swap_pages = get_nr_swap_pages();

	if (mem_cgroup_disabled() || do_memsw_account())
		return nr_swap_pages;
	for (; memcg != root_mem_cgroup; memcg = parent_mem_cgroup(memcg))
		nr_swap_pages = min_t(long, nr_swap_pages,
				      READ_ONCE(memcg->swap.max) -
				      page_counter_read(&memcg->swap));
	return nr_swap_pages;
}

struct mem_cgroup_get_nr_swap_pages__HEXSHA { void _b25806dcd3d5; };
struct mem_cgroup_get_nr_swap_pages__ATTRS { };
