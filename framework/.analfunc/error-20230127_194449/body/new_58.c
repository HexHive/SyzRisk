struct evsel *evsel__clone(struct evsel *orig)
{
	struct evsel *evsel;

	BUG_ON(orig->core.fd);
	BUG_ON(orig->counts);
	BUG_ON(orig->priv);
	BUG_ON(orig->per_pkg_mask);

	/* cannot handle BPF objects for now */
	if (orig->bpf_obj)
		return NULL;

	evsel = evsel__new(&orig->core.attr);
	if (evsel == NULL)
		return NULL;

	evsel->core.cpus = perf_cpu_map__get(orig->core.cpus);
	evsel->core.own_cpus = perf_cpu_map__get(orig->core.own_cpus);
	evsel->core.threads = perf_thread_map__get(orig->core.threads);
	evsel->core.nr_members = orig->core.nr_members;
	evsel->core.system_wide = orig->core.system_wide;
	evsel->core.requires_cpu = orig->core.requires_cpu;

	if (orig->name) {
		evsel->name = strdup(orig->name);
		if (evsel->name == NULL)
			goto out_err;
	}
	if (orig->group_name) {
		evsel->group_name = strdup(orig->group_name);
		if (evsel->group_name == NULL)
			goto out_err;
	}
	if (orig->pmu_name) {
		evsel->pmu_name = strdup(orig->pmu_name);
		if (evsel->pmu_name == NULL)
			goto out_err;
	}
	if (orig->filter) {
		evsel->filter = strdup(orig->filter);
		if (evsel->filter == NULL)
			goto out_err;
	}
	if (orig->metric_id) {
		evsel->metric_id = strdup(orig->metric_id);
		if (evsel->metric_id == NULL)
			goto out_err;
	}
	evsel->cgrp = cgroup__get(orig->cgrp);
	evsel->tp_format = orig->tp_format;
	evsel->handler = orig->handler;
	evsel->core.leader = orig->core.leader;

	evsel->max_events = orig->max_events;
	evsel->tool_event = orig->tool_event;
	free((char *)evsel->unit);
	evsel->unit = strdup(orig->unit);
	if (evsel->unit == NULL)
		goto out_err;

	evsel->scale = orig->scale;
	evsel->snapshot = orig->snapshot;
	evsel->per_pkg = orig->per_pkg;
	evsel->percore = orig->percore;
	evsel->precise_max = orig->precise_max;
	evsel->use_uncore_alias = orig->use_uncore_alias;
	evsel->is_libpfm_event = orig->is_libpfm_event;

	evsel->exclude_GH = orig->exclude_GH;
	evsel->sample_read = orig->sample_read;
	evsel->auto_merge_stats = orig->auto_merge_stats;
	evsel->collect_stat = orig->collect_stat;
	evsel->weak_group = orig->weak_group;
	evsel->use_config_name = orig->use_config_name;
	evsel->pmu = orig->pmu;

	if (evsel__copy_config_terms(evsel, orig) < 0)
		goto out_err;

	return evsel;

out_err:
	evsel__delete(evsel);
	return NULL;
}

struct evsel__clone__HEXSHA { void _f7400262ea21; };
struct evsel__clone__ATTRS { };
