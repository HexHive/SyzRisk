static struct evsel *
__add_event(struct list_head *list, int *idx,
	    struct perf_event_attr *attr,
	    bool init_attr,
	    const char *name, const char *metric_id, struct perf_pmu *pmu,
	    struct list_head *config_terms, bool auto_merge_stats,
	    const char *cpu_list)
{
	struct evsel *evsel;
	struct perf_cpu_map *cpus = pmu ? perf_cpu_map__get(pmu->cpus) :
			       cpu_list ? perf_cpu_map__new(cpu_list) : NULL;

	if (pmu)
		perf_pmu__warn_invalid_formats(pmu);

	if (pmu && attr->type == PERF_TYPE_RAW)
		perf_pmu__warn_invalid_config(pmu, attr->config, name);

	if (init_attr)
		event_attr_init(attr);

	evsel = evsel__new_idx(attr, *idx);
	if (!evsel) {
		perf_cpu_map__put(cpus);
		return NULL;
	}

	(*idx)++;
	evsel->core.cpus = cpus;
	evsel->core.own_cpus = perf_cpu_map__get(cpus);
	evsel->core.requires_cpu = pmu ? pmu->is_uncore : false;
	evsel->auto_merge_stats = auto_merge_stats;
	evsel->pmu = pmu;

	if (name)
		evsel->name = strdup(name);

	if (metric_id)
		evsel->metric_id = strdup(metric_id);

	if (config_terms)
		list_splice_init(config_terms, &evsel->config_terms);

	if (list)
		list_add_tail(&evsel->core.node, list);

	return evsel;
}

struct __add_event__HEXSHA { void _f7400262ea21; };
struct __add_event__ATTRS { };
