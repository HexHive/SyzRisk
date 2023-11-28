static void
format_mca_init_stack(void *mca_data, unsigned long offset,
		const char *type, int cpu)
{
	struct task_struct *p = (struct task_struct *)((char *)mca_data + offset);
	struct thread_info *ti;
	memset(p, 0, KERNEL_STACK_SIZE);
	ti = task_thread_info(p);
	ti->flags = _TIF_MCA_INIT;
	ti->preempt_count = 1;
	ti->task = p;
	ti->cpu = cpu;
	p->stack = ti;
	p->__state = TASK_UNINTERRUPTIBLE;
	cpumask_set_cpu(cpu, &p->cpus_mask);
	INIT_LIST_HEAD(&p->tasks);
	p->parent = p->real_parent = p->group_leader = p;
	INIT_LIST_HEAD(&p->children);
	INIT_LIST_HEAD(&p->sibling);
	strncpy(p->comm, type, sizeof(p->comm)-1);
}

struct format_mca_init_stack__HEXSHA { void _95e9a8552e85; };
struct format_mca_init_stack__ATTRS { };
