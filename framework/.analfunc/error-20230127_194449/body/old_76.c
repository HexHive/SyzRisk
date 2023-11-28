static inline bool sched_group_cookie_match(struct rq *rq,
					    struct task_struct *p,
					    struct sched_group *group)
{
	int cpu;

	/* Ignore cookie match if core scheduler is not enabled on the CPU. */
	if (!sched_core_enabled(rq))
		return true;

	for_each_cpu_and(cpu, sched_group_span(group), p->cpus_ptr) {
		if (sched_core_cookie_match(rq, p))
			return true;
	}
	return false;
}

struct sched_group_cookie_match__HEXSHA { void _e705968dd687; };
struct sched_group_cookie_match__ATTRS { };
