int aa_profile_af_perm(struct aa_profile *profile, struct common_audit_data *sa,
		       u32 request, u16 family, int type)
{
	struct aa_perms perms = { };
	unsigned int state;
	__be16 buffer[2];

	AA_BUG(family >= AF_MAX);
	AA_BUG(type < 0 || type >= SOCK_MAX);

	if (profile_unconfined(profile))
		return 0;
	state = PROFILE_MEDIATES(profile, AA_CLASS_NET);
	if (!state)
		return 0;

	buffer[0] = cpu_to_be16(family);
	buffer[1] = cpu_to_be16((u16) type);
	state = aa_dfa_match_len(profile->policy.dfa, state, (char *) &buffer,
				 4);
	perms = *aa_lookup_perms(&profile->policy, state);
	aa_apply_modes_to_perms(profile, &perms);

	return aa_check_perms(profile, &perms, request, sa, audit_net_cb);
}

struct aa_profile_af_perm__HEXSHA { void _e844fe9b51c9; };
struct aa_profile_af_perm__ATTRS { };
