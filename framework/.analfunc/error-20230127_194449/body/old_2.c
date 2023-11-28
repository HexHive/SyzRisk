static int do_match_mnt(struct aa_policydb *policy, unsigned int start,
			const char *mntpnt, const char *devname,
			const char *type, unsigned long flags,
			void *data, bool binary, struct aa_perms *perms)
{
	unsigned int state;

	AA_BUG(!policy);
	AA_BUG(!policy->dfa);
	AA_BUG(!policy->perms);
	AA_BUG(!perms);

	state = aa_dfa_match(policy->dfa, start, mntpnt);
	state = aa_dfa_null_transition(policy->dfa, state);
	if (!state)
		return 1;

	if (devname)
		state = aa_dfa_match(policy->dfa, state, devname);
	state = aa_dfa_null_transition(policy->dfa, state);
	if (!state)
		return 2;

	if (type)
		state = aa_dfa_match(policy->dfa, state, type);
	state = aa_dfa_null_transition(policy->dfa, state);
	if (!state)
		return 3;

	state = match_mnt_flags(policy->dfa, state, flags);
	if (!state)
		return 4;
	*perms = *aa_lookup_perms(policy->perms, state);
	if (perms->allow & AA_MAY_MOUNT)
		return 0;

	/* only match data if not binary and the DFA flags data is expected */
	if (data && !binary && (perms->allow & AA_MNT_CONT_MATCH)) {
		state = aa_dfa_null_transition(policy->dfa, state);
		if (!state)
			return 4;

		state = aa_dfa_match(policy->dfa, state, data);
		if (!state)
			return 5;
		*perms = *aa_lookup_perms(policy->perms, state);
		if (perms->allow & AA_MAY_MOUNT)
			return 0;
	}

	/* failed at perms check, don't confuse with flags match */
	return 6;
}

struct do_match_mnt__HEXSHA { void _e844fe9b51c9; };
struct do_match_mnt__ATTRS { };
