struct perf_pmu *evsel__find_pmu(struct evsel *evsel)
{
	struct perf_pmu *pmu = NULL;

	if (evsel->pmu)
		return evsel->pmu;

	while ((pmu = perf_pmu__scan(pmu)) != NULL) {
		if (pmu->type == evsel->core.attr.type)
			break;
	}

	evsel->pmu = pmu;
	return pmu;
}

struct evsel__find_pmu__HEXSHA { void _f7400262ea21; };
struct evsel__find_pmu__ATTRS { };
