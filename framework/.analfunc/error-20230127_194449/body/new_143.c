static int paiext_push_sample(void)
{
	struct paiext_mapptr *mp = this_cpu_ptr(paiext_root.mapptr);
	struct paiext_map *cpump = mp->mapptr;
	struct perf_event *event = cpump->event;
	struct perf_sample_data data;
	struct perf_raw_record raw;
	struct pt_regs regs;
	size_t rawsize;
	int overflow;

	rawsize = paiext_copy(cpump);
	if (!rawsize)			/* No incremented counters */
		return 0;

	/* Setup perf sample */
	memset(&regs, 0, sizeof(regs));
	memset(&raw, 0, sizeof(raw));
	memset(&data, 0, sizeof(data));
	perf_sample_data_init(&data, 0, event->hw.last_period);
	if (event->attr.sample_type & PERF_SAMPLE_TID) {
		data.tid_entry.pid = task_tgid_nr(current);
		data.tid_entry.tid = task_pid_nr(current);
	}
	if (event->attr.sample_type & PERF_SAMPLE_TIME)
		data.time = event->clock();
	if (event->attr.sample_type & (PERF_SAMPLE_ID | PERF_SAMPLE_IDENTIFIER))
		data.id = event->id;
	if (event->attr.sample_type & PERF_SAMPLE_CPU)
		data.cpu_entry.cpu = smp_processor_id();
	if (event->attr.sample_type & PERF_SAMPLE_RAW) {
		raw.frag.size = rawsize;
		raw.frag.data = cpump->save;
		raw.size = raw.frag.size;
		data.raw = &raw;
		data.sample_flags |= PERF_SAMPLE_RAW;
	}

	overflow = perf_event_overflow(event, &data, &regs);
	perf_event_update_userpage(event);
	/* Clear lowcore area after read */
	memset(cpump->area, 0, PAIE1_CTRBLOCK_SZ);
	return overflow;
}

struct paiext_push_sample__HEXSHA { void _8b1e6a3fb3fe; };
struct paiext_push_sample__ATTRS { };
