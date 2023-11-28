static struct aa_label *build_pivotroot(struct aa_profile *profile,
					const struct path *new_path,
					char *new_buffer,
					const struct path *old_path,
					char *old_buffer)
{
	const char *old_name, *new_name = NULL, *info = NULL;
	const char *trans_name = NULL;
	struct aa_perms perms = { };
	unsigned int state;
	int error;

	AA_BUG(!profile);
	AA_BUG(!new_path);
	AA_BUG(!old_path);

	if (profile_unconfined(profile) ||
	    !PROFILE_MEDIATES(profile, AA_CLASS_MOUNT))
		return aa_get_newest_label(&profile->label);

	error = aa_path_name(old_path, path_flags(profile, old_path),
			     old_buffer, &old_name, &info,
			     profile->disconnected);
	if (error)
		goto audit;
	error = aa_path_name(new_path, path_flags(profile, new_path),
			     new_buffer, &new_name, &info,
			     profile->disconnected);
	if (error)
		goto audit;

	error = -EACCES;
	state = aa_dfa_match(profile->policy.dfa,
			     profile->policy.start[AA_CLASS_MOUNT],
			     new_name);
	state = aa_dfa_null_transition(profile->policy.dfa, state);
	state = aa_dfa_match(profile->policy.dfa, state, old_name);
	perms = *aa_lookup_perms(&profile->policy, state);

	if (AA_MAY_PIVOTROOT & perms.allow)
		error = 0;

audit:
	error = audit_mount(profile, OP_PIVOTROOT, new_name, old_name,
			    NULL, trans_name, 0, NULL, AA_MAY_PIVOTROOT,
			    &perms, info, error);
	if (error)
		return ERR_PTR(error);

	return aa_get_newest_label(&profile->label);
}

struct build_pivotroot__HEXSHA { void _e844fe9b51c9; };
struct build_pivotroot__ATTRS { };
