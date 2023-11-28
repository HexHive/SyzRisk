static int label_compound_match(struct aa_profile *profile,
				struct aa_label *label,
				unsigned int state, bool subns, u32 request,
				struct aa_perms *perms)
{
	struct aa_profile *tp;
	struct label_it i;

	/* find first subcomponent that is visible */
	label_for_each(i, label, tp) {
		if (!aa_ns_visible(profile->ns, tp->ns, subns))
			continue;
		state = match_component(profile, tp, state);
		if (!state)
			goto fail;
		goto next;
	}

	/* no component visible */
	*perms = allperms;
	return 0;

next:
	label_for_each_cont(i, label, tp) {
		if (!aa_ns_visible(profile->ns, tp->ns, subns))
			continue;
		state = aa_dfa_match(profile->policy.dfa, state, "//&");
		state = match_component(profile, tp, state);
		if (!state)
			goto fail;
	}
	*perms = *aa_lookup_perms(profile->policy.perms, state);
	aa_apply_modes_to_perms(profile, perms);
	if ((perms->allow & request) != request)
		return -EACCES;

	return 0;

fail:
	*perms = nullperms;
	return state;
}

struct label_compound_match__HEXSHA { void _e844fe9b51c9; };
struct label_compound_match__ATTRS { };
