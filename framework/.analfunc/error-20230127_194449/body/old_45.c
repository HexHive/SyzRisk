bool evsel__detect_missing_features(struct evsel *evsel)
{
	/*
	 * Must probe features in the order they were added to the
	 * perf_event_attr interface.
	 */
	if (!perf_missing_features.read_lost &&
	    (evsel->core.attr.read_format & PERF_FORMAT_LOST)) {
		perf_missing_features.read_lost = true;
		pr_debug2("switching off PERF_FORMAT_LOST support\n");
		return true;
	} else if (!perf_missing_features.weight_struct &&
	    (evsel->core.attr.sample_type & PERF_SAMPLE_WEIGHT_STRUCT)) {
		perf_missing_features.weight_struct = true;
		pr_debug2("switching off weight struct support\n");
		return true;
	} else if (!perf_missing_features.code_page_size &&
	    (evsel->core.attr.sample_type & PERF_SAMPLE_CODE_PAGE_SIZE)) {
		perf_missing_features.code_page_size = true;
		pr_debug2_peo("Kernel has no PERF_SAMPLE_CODE_PAGE_SIZE support, bailing out\n");
		return false;
	} else if (!perf_missing_features.data_page_size &&
	    (evsel->core.attr.sample_type & PERF_SAMPLE_DATA_PAGE_SIZE)) {
		perf_missing_features.data_page_size = true;
		pr_debug2_peo("Kernel has no PERF_SAMPLE_DATA_PAGE_SIZE support, bailing out\n");
		return false;
	} else if (!perf_missing_features.cgroup && evsel->core.attr.cgroup) {
		perf_missing_features.cgroup = true;
		pr_debug2_peo("Kernel has no cgroup sampling support, bailing out\n");
		return false;
	} else if (!perf_missing_features.branch_hw_idx &&
	    (evsel->core.attr.branch_sample_type & PERF_SAMPLE_BRANCH_HW_INDEX)) {
		perf_missing_features.branch_hw_idx = true;
		pr_debug2("switching off branch HW index support\n");
		return true;
	} else if (!perf_missing_features.aux_output && evsel->core.attr.aux_output) {
		perf_missing_features.aux_output = true;
		pr_debug2_peo("Kernel has no attr.aux_output support, bailing out\n");
		return false;
	} else if (!perf_missing_features.bpf && evsel->core.attr.bpf_event) {
		perf_missing_features.bpf = true;
		pr_debug2_peo("switching off bpf_event\n");
		return true;
	} else if (!perf_missing_features.ksymbol && evsel->core.attr.ksymbol) {
		perf_missing_features.ksymbol = true;
		pr_debug2_peo("switching off ksymbol\n");
		return true;
	} else if (!perf_missing_features.write_backward && evsel->core.attr.write_backward) {
		perf_missing_features.write_backward = true;
		pr_debug2_peo("switching off write_backward\n");
		return false;
	} else if (!perf_missing_features.clockid_wrong && evsel->core.attr.use_clockid) {
		perf_missing_features.clockid_wrong = true;
		pr_debug2_peo("switching off clockid\n");
		return true;
	} else if (!perf_missing_features.clockid && evsel->core.attr.use_clockid) {
		perf_missing_features.clockid = true;
		pr_debug2_peo("switching off use_clockid\n");
		return true;
	} else if (!perf_missing_features.cloexec && (evsel->open_flags & PERF_FLAG_FD_CLOEXEC)) {
		perf_missing_features.cloexec = true;
		pr_debug2_peo("switching off cloexec flag\n");
		return true;
	} else if (!perf_missing_features.mmap2 && evsel->core.attr.mmap2) {
		perf_missing_features.mmap2 = true;
		pr_debug2_peo("switching off mmap2\n");
		return true;
	} else if ((evsel->core.attr.exclude_guest || evsel->core.attr.exclude_host) &&
		   (evsel->pmu == NULL || evsel->pmu->missing_features.exclude_guest)) {
		if (evsel->pmu == NULL) {
			evsel->pmu = evsel__find_pmu(evsel);
			if (evsel->pmu)
				evsel->pmu->missing_features.exclude_guest = true;
			else {
				/* we cannot find PMU, disable attrs now */
				evsel->core.attr.exclude_host = false;
				evsel->core.attr.exclude_guest = false;
			}
		}

		if (evsel->exclude_GH) {
			pr_debug2_peo("PMU has no exclude_host/guest support, bailing out\n");
			return false;
		}
		if (!perf_missing_features.exclude_guest) {
			perf_missing_features.exclude_guest = true;
			pr_debug2_peo("switching off exclude_guest, exclude_host\n");
		}
		return true;
	} else if (!perf_missing_features.sample_id_all) {
		perf_missing_features.sample_id_all = true;
		pr_debug2_peo("switching off sample_id_all\n");
		return true;
	} else if (!perf_missing_features.lbr_flags &&
			(evsel->core.attr.branch_sample_type &
			 (PERF_SAMPLE_BRANCH_NO_CYCLES |
			  PERF_SAMPLE_BRANCH_NO_FLAGS))) {
		perf_missing_features.lbr_flags = true;
		pr_debug2_peo("switching off branch sample type no (cycles/flags)\n");
		return true;
	} else if (!perf_missing_features.group_read &&
		    evsel->core.attr.inherit &&
		   (evsel->core.attr.read_format & PERF_FORMAT_GROUP) &&
		   evsel__is_group_leader(evsel)) {
		perf_missing_features.group_read = true;
		pr_debug2_peo("switching off group read\n");
		return true;
	} else {
		return false;
	}
}

struct evsel__detect_missing_features__HEXSHA { void _f7400262ea21; };
struct evsel__detect_missing_features__ATTRS { };
