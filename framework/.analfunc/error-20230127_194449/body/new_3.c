static int verify_profile(struct aa_profile *profile)
{
	if ((profile->file.dfa &&
	     !verify_dfa_xindex(profile->file.dfa,
				profile->file.trans.size)) ||
	    (profile->policy.dfa &&
	     !verify_dfa_xindex(profile->policy.dfa,
				profile->policy.trans.size))) {
		audit_iface(profile, NULL, NULL,
			    "Unpack: Invalid named transition", NULL, -EPROTO);
		return -EPROTO;
	}

	return 0;
}

struct verify_profile__HEXSHA { void _e844fe9b51c9; };
struct verify_profile__ATTRS { };
