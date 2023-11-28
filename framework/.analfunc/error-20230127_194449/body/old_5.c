static int label_components_match(struct aa_profile *profile,
				  struct aa_label *label, unsigned int start,
				  bool subns, u32 request,
				  struct aa_perms *perms)
{
	struct aa_profile *tp;
	struct label_it i;
	struct aa_perms tmp;
	unsigned int state = 0;

	/* find first subcomponent to test */
	label_for_each(i, label, tp) {
		if (!aa_ns_visible(profile->ns, tp->ns, subns))
			continue;
		state = match_component(profile, tp, start);
		if (!state)
			goto fail;
		goto next;
	}

	/* no subcomponents visible - no change in perms */
	return 0;

next:
	tmp = *aa_lookup_perms(profile->policy.perms, state);
	aa_apply_modes_to_perms(profile, &tmp);
	aa_perms_accum(perms, &tmp);
	label_for_each_cont(i, label, tp) {
		if (!aa_ns_visible(profile->ns, tp->ns, subns))
			continue;
		state = match_component(profile, tp, start);
		if (!state)
			goto fail;
		tmp = *aa_lookup_perms(profile->policy.perms, state);
		aa_apply_modes_to_perms(profile, &tmp);
		aa_perms_accum(perms, &tmp);
	}

	if ((perms->allow & request) != request)
		return -EACCES;

	return 0;

fail:
	*perms = nullperms;
	return -EACCES;
}

struct label_components_match__HEXSHA { void _e844fe9b51c9; };
struct label_components_match__ATTRS { };
