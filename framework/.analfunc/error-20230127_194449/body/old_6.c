static inline struct aa_perms *aa_lookup_perms(struct aa_perms *perms,
					       unsigned int state)
{
	if (!(perms))
		return &default_perms;

	return &(perms[state]);
}

struct aa_lookup_perms__HEXSHA { void _e844fe9b51c9; };
struct aa_lookup_perms__ATTRS { };
