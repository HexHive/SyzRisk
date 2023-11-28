static inline struct aa_perms *aa_lookup_perms(struct aa_policydb *policy,
					       unsigned int state)
{
	unsigned int index = ACCEPT_TABLE(policy->dfa)[state];

	if (!(policy->perms))
		return &default_perms;

	return &(policy->perms[index]);
}

struct aa_lookup_perms__HEXSHA { void _e844fe9b51c9; };
struct aa_lookup_perms__ATTRS { };
