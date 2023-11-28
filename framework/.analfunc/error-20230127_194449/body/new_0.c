static void profile_query_cb(struct aa_profile *profile, struct aa_perms *perms,
			     const char *match_str, size_t match_len)
{
	struct aa_perms tmp = { };
	struct aa_dfa *dfa;
	unsigned int state = 0;

	if (profile_unconfined(profile))
		return;
	if (profile->file.dfa && *match_str == AA_CLASS_FILE) {
		dfa = profile->file.dfa;
		state = aa_dfa_match_len(dfa,
					 profile->file.start[AA_CLASS_FILE],
					 match_str + 1, match_len - 1);
		if (state) {
			struct path_cond cond = { };

			tmp = *(aa_lookup_fperms(&(profile->file), state, &cond));
		}
	} else if (profile->policy.dfa) {
		if (!PROFILE_MEDIATES(profile, *match_str))
			return;	/* no change to current perms */
		dfa = profile->policy.dfa;
		state = aa_dfa_match_len(dfa, profile->policy.start[0],
					 match_str, match_len);
		if (state)
			tmp = *aa_lookup_perms(&profile->policy, state);
	}
	aa_apply_modes_to_perms(profile, &tmp);
	aa_perms_accum_raw(perms, &tmp);
}

struct profile_query_cb__HEXSHA { void _e844fe9b51c9; };
struct profile_query_cb__ATTRS { };
