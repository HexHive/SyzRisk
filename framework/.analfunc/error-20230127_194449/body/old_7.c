static int profile_umount(struct aa_profile *profile, const struct path *path,
			  char *buffer)
{
	struct aa_perms perms = { };
	const char *name = NULL, *info = NULL;
	unsigned int state;
	int error;

	AA_BUG(!profile);
	AA_BUG(!path);

	if (!PROFILE_MEDIATES(profile, AA_CLASS_MOUNT))
		return 0;

	error = aa_path_name(path, path_flags(profile, path), buffer, &name,
			     &info, profile->disconnected);
	if (error)
		goto audit;

	state = aa_dfa_match(profile->policy.dfa,
			     profile->policy.start[AA_CLASS_MOUNT],
			     name);
	perms = *aa_lookup_perms(profile->policy.perms, state);
	if (AA_MAY_UMOUNT & ~perms.allow)
		error = -EACCES;

audit:
	return audit_mount(profile, OP_UMOUNT, name, NULL, NULL, NULL, 0, NULL,
			   AA_MAY_UMOUNT, &perms, info, error);
}

struct profile_umount__HEXSHA { void _e844fe9b51c9; };
struct profile_umount__ATTRS { };
