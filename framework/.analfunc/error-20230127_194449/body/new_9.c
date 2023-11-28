static struct aa_profile *unpack_profile(struct aa_ext *e, char **ns_name)
{
	struct aa_profile *profile = NULL;
	const char *tmpname, *tmpns = NULL, *name = NULL;
	const char *info = "failed to unpack profile";
	size_t ns_len;
	struct rhashtable_params params = { 0 };
	char *key = NULL;
	struct aa_data *data;
	int i, error = -EPROTO;
	kernel_cap_t tmpcap;
	u32 tmp;

	*ns_name = NULL;

	/* check that we have the right struct being passed */
	if (!unpack_nameX(e, AA_STRUCT, "profile"))
		goto fail;
	if (!unpack_str(e, &name, NULL))
		goto fail;
	if (*name == '\0')
		goto fail;

	tmpname = aa_splitn_fqname(name, strlen(name), &tmpns, &ns_len);
	if (tmpns) {
		*ns_name = kstrndup(tmpns, ns_len, GFP_KERNEL);
		if (!*ns_name) {
			info = "out of memory";
			goto fail;
		}
		name = tmpname;
	}

	profile = aa_alloc_profile(name, NULL, GFP_KERNEL);
	if (!profile)
		return ERR_PTR(-ENOMEM);

	/* profile renaming is optional */
	(void) unpack_str(e, &profile->rename, "rename");

	/* attachment string is optional */
	(void) unpack_str(e, &profile->attach, "attach");

	/* xmatch is optional and may be NULL */
	profile->xmatch.dfa = unpack_dfa(e);
	if (IS_ERR(profile->xmatch.dfa)) {
		error = PTR_ERR(profile->xmatch.dfa);
		profile->xmatch.dfa = NULL;
		info = "bad xmatch";
		goto fail;
	}
	/* neither xmatch_len not xmatch_perms are optional if xmatch is set */
	if (profile->xmatch.dfa) {
		if (!unpack_u32(e, &tmp, NULL)) {
			info = "missing xmatch len";
			goto fail;
		}
		profile->xmatch_len = tmp;
		profile->xmatch.start[AA_CLASS_XMATCH] = DFA_START;
		profile->xmatch.perms = compute_xmatch_perms(profile->xmatch.dfa);
		if (!profile->xmatch.perms) {
			info = "failed to convert xmatch permission table";
			goto fail;
		}
		remap_dfa_accept(profile->xmatch.dfa, 1);
	}

	/* disconnected attachment string is optional */
	(void) unpack_str(e, &profile->disconnected, "disconnected");

	/* per profile debug flags (complain, audit) */
	if (!unpack_nameX(e, AA_STRUCT, "flags")) {
		info = "profile missing flags";
		goto fail;
	}
	info = "failed to unpack profile flags";
	if (!unpack_u32(e, &tmp, NULL))
		goto fail;
	if (tmp & PACKED_FLAG_HAT)
		profile->label.flags |= FLAG_HAT;
	if (tmp & PACKED_FLAG_DEBUG1)
		profile->label.flags |= FLAG_DEBUG1;
	if (tmp & PACKED_FLAG_DEBUG2)
		profile->label.flags |= FLAG_DEBUG2;
	if (!unpack_u32(e, &tmp, NULL))
		goto fail;
	if (tmp == PACKED_MODE_COMPLAIN || (e->version & FORCE_COMPLAIN_FLAG)) {
		profile->mode = APPARMOR_COMPLAIN;
	} else if (tmp == PACKED_MODE_ENFORCE) {
		profile->mode = APPARMOR_ENFORCE;
	} else if (tmp == PACKED_MODE_KILL) {
		profile->mode = APPARMOR_KILL;
	} else if (tmp == PACKED_MODE_UNCONFINED) {
		profile->mode = APPARMOR_UNCONFINED;
		profile->label.flags |= FLAG_UNCONFINED;
	} else {
		goto fail;
	}
	if (!unpack_u32(e, &tmp, NULL))
		goto fail;
	if (tmp)
		profile->audit = AUDIT_ALL;

	if (!unpack_nameX(e, AA_STRUCTEND, NULL))
		goto fail;

	/* path_flags is optional */
	if (unpack_u32(e, &profile->path_flags, "path_flags"))
		profile->path_flags |= profile->label.flags &
			PATH_MEDIATE_DELETED;
	else
		/* set a default value if path_flags field is not present */
		profile->path_flags = PATH_MEDIATE_DELETED;

	info = "failed to unpack profile capabilities";
	if (!unpack_u32(e, &(profile->caps.allow.cap[0]), NULL))
		goto fail;
	if (!unpack_u32(e, &(profile->caps.audit.cap[0]), NULL))
		goto fail;
	if (!unpack_u32(e, &(profile->caps.quiet.cap[0]), NULL))
		goto fail;
	if (!unpack_u32(e, &tmpcap.cap[0], NULL))
		goto fail;

	info = "failed to unpack upper profile capabilities";
	if (unpack_nameX(e, AA_STRUCT, "caps64")) {
		/* optional upper half of 64 bit caps */
		if (!unpack_u32(e, &(profile->caps.allow.cap[1]), NULL))
			goto fail;
		if (!unpack_u32(e, &(profile->caps.audit.cap[1]), NULL))
			goto fail;
		if (!unpack_u32(e, &(profile->caps.quiet.cap[1]), NULL))
			goto fail;
		if (!unpack_u32(e, &(tmpcap.cap[1]), NULL))
			goto fail;
		if (!unpack_nameX(e, AA_STRUCTEND, NULL))
			goto fail;
	}

	info = "failed to unpack extended profile capabilities";
	if (unpack_nameX(e, AA_STRUCT, "capsx")) {
		/* optional extended caps mediation mask */
		if (!unpack_u32(e, &(profile->caps.extended.cap[0]), NULL))
			goto fail;
		if (!unpack_u32(e, &(profile->caps.extended.cap[1]), NULL))
			goto fail;
		if (!unpack_nameX(e, AA_STRUCTEND, NULL))
			goto fail;
	}

	if (!unpack_xattrs(e, profile)) {
		info = "failed to unpack profile xattrs";
		goto fail;
	}

	if (!unpack_rlimits(e, profile)) {
		info = "failed to unpack profile rlimits";
		goto fail;
	}

	if (!unpack_secmark(e, profile)) {
		info = "failed to unpack profile secmark rules";
		goto fail;
	}

	if (unpack_nameX(e, AA_STRUCT, "policydb")) {
		/* generic policy dfa - optional and may be NULL */
		info = "failed to unpack policydb";
		profile->policy.dfa = unpack_dfa(e);
		if (IS_ERR(profile->policy.dfa)) {
			error = PTR_ERR(profile->policy.dfa);
			profile->policy.dfa = NULL;
			goto fail;
		} else if (!profile->policy.dfa) {
			error = -EPROTO;
			goto fail;
		}
		if (!unpack_u32(e, &profile->policy.start[0], "start"))
			/* default start state */
			profile->policy.start[0] = DFA_START;
		/* setup class index */
		for (i = AA_CLASS_FILE; i <= AA_CLASS_LAST; i++) {
			profile->policy.start[i] =
				aa_dfa_next(profile->policy.dfa,
					    profile->policy.start[0],
					    i);
		}
		if (!unpack_nameX(e, AA_STRUCTEND, NULL))
			goto fail;
		profile->policy.perms = compute_perms(profile->policy.dfa);
		if (!profile->policy.perms) {
			info = "failed to remap policydb permission table";
			goto fail;
		}
		/* Do not remap internal dfas */
		remap_dfa_accept(profile->policy.dfa, 1);
	} else
		profile->policy.dfa = aa_get_dfa(nulldfa);

	/* get file rules */
	profile->file.dfa = unpack_dfa(e);
	if (IS_ERR(profile->file.dfa)) {
		error = PTR_ERR(profile->file.dfa);
		profile->file.dfa = NULL;
		info = "failed to unpack profile file rules";
		goto fail;
	} else if (profile->file.dfa) {
		if (!unpack_u32(e, &profile->file.start[AA_CLASS_FILE],
				"dfa_start"))
			/* default start state */
			profile->file.start[AA_CLASS_FILE] = DFA_START;
		profile->file.perms = compute_fperms(profile->file.dfa);
		if (!profile->file.perms) {
			info = "failed to remap file permission table";
			goto fail;
		}
		remap_dfa_accept(profile->file.dfa, 2);
		if (!unpack_trans_table(e, profile)) {
			info = "failed to unpack profile transition table";
			goto fail;
		}
	} else if (profile->policy.dfa &&
		   profile->policy.start[AA_CLASS_FILE]) {
		profile->file.dfa = aa_get_dfa(profile->policy.dfa);
		profile->file.start[AA_CLASS_FILE] = profile->policy.start[AA_CLASS_FILE];
	} else
		profile->file.dfa = aa_get_dfa(nulldfa);

	if (unpack_nameX(e, AA_STRUCT, "data")) {
		info = "out of memory";
		profile->data = kzalloc(sizeof(*profile->data), GFP_KERNEL);
		if (!profile->data)
			goto fail;

		params.nelem_hint = 3;
		params.key_len = sizeof(void *);
		params.key_offset = offsetof(struct aa_data, key);
		params.head_offset = offsetof(struct aa_data, head);
		params.hashfn = strhash;
		params.obj_cmpfn = datacmp;

		if (rhashtable_init(profile->data, &params)) {
			info = "failed to init key, value hash table";
			goto fail;
		}

		while (unpack_strdup(e, &key, NULL)) {
			data = kzalloc(sizeof(*data), GFP_KERNEL);
			if (!data) {
				kfree_sensitive(key);
				goto fail;
			}

			data->key = key;
			data->size = unpack_blob(e, &data->data, NULL);
			data->data = kvmemdup(data->data, data->size);
			if (data->size && !data->data) {
				kfree_sensitive(data->key);
				kfree_sensitive(data);
				goto fail;
			}

			rhashtable_insert_fast(profile->data, &data->head,
					       profile->data->p);
		}

		if (!unpack_nameX(e, AA_STRUCTEND, NULL)) {
			info = "failed to unpack end of key, value data table";
			goto fail;
		}
	}

	if (!unpack_nameX(e, AA_STRUCTEND, NULL)) {
		info = "failed to unpack end of profile";
		goto fail;
	}

	return profile;

fail:
	if (profile)
		name = NULL;
	else if (!name)
		name = "unknown";
	audit_iface(profile, NULL, name, info, e, error);
	aa_free_profile(profile);

	return ERR_PTR(error);
}

struct unpack_profile__HEXSHA { void _e844fe9b51c9; };
struct unpack_profile__ATTRS { };
