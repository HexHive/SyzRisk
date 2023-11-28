ssize_t aa_replace_profiles(struct aa_ns *policy_ns, struct aa_label *label,
			    u32 mask, struct aa_loaddata *udata)
{
	const char *ns_name = NULL, *info = NULL;
	struct aa_ns *ns = NULL;
	struct aa_load_ent *ent, *tmp;
	struct aa_loaddata *rawdata_ent;
	const char *op;
	ssize_t count, error;
	LIST_HEAD(lh);

	op = mask & AA_MAY_REPLACE_POLICY ? OP_PROF_REPL : OP_PROF_LOAD;
	aa_get_loaddata(udata);
	/* released below */
	error = aa_unpack(udata, &lh, &ns_name);
	if (error)
		goto out;

	/* ensure that profiles are all for the same ns
	 * TODO: update locking to remove this constaint. All profiles in
	 *       the load set must succeed as a set or the load will
	 *       fail. Sort ent list and take ns locks in hierarchy order
	 */
	count = 0;
	list_for_each_entry(ent, &lh, list) {
		if (ns_name) {
			if (ent->ns_name &&
			    strcmp(ent->ns_name, ns_name) != 0) {
				info = "policy load has mixed namespaces";
				error = -EACCES;
				goto fail;
			}
		} else if (ent->ns_name) {
			if (count) {
				info = "policy load has mixed namespaces";
				error = -EACCES;
				goto fail;
			}
			ns_name = ent->ns_name;
		} else
			count++;
	}
	if (ns_name) {
		ns = aa_prepare_ns(policy_ns ? policy_ns : labels_ns(label),
				   ns_name);
		if (IS_ERR(ns)) {
			op = OP_PROF_LOAD;
			info = "failed to prepare namespace";
			error = PTR_ERR(ns);
			ns = NULL;
			ent = NULL;
			goto fail;
		}
	} else
		ns = aa_get_ns(policy_ns ? policy_ns : labels_ns(label));

	mutex_lock_nested(&ns->lock, ns->level);
	/* check for duplicate rawdata blobs: space and file dedup */
	if (!list_empty(&ns->rawdata_list)) {
		list_for_each_entry(rawdata_ent, &ns->rawdata_list, list) {
			if (aa_rawdata_eq(rawdata_ent, udata)) {
				struct aa_loaddata *tmp;

				tmp = __aa_get_loaddata(rawdata_ent);
				/* check we didn't fail the race */
				if (tmp) {
					aa_put_loaddata(udata);
					udata = tmp;
					break;
				}
			}
		}
	}
	/* setup parent and ns info */
	list_for_each_entry(ent, &lh, list) {
		struct aa_policy *policy;
		struct aa_profile *p;

		if (aa_g_export_binary)
			ent->new->rawdata = aa_get_loaddata(udata);
		error = __lookup_replace(ns, ent->new->base.hname,
					 !(mask & AA_MAY_REPLACE_POLICY),
					 &ent->old, &info);
		if (error)
			goto fail_lock;

		if (ent->new->rename) {
			error = __lookup_replace(ns, ent->new->rename,
						!(mask & AA_MAY_REPLACE_POLICY),
						&ent->rename, &info);
			if (error)
				goto fail_lock;
		}

		/* released when @new is freed */
		ent->new->ns = aa_get_ns(ns);

		if (ent->old || ent->rename)
			continue;

		/* no ref on policy only use inside lock */
		p = NULL;
		policy = __lookup_parent(ns, ent->new->base.hname);
		if (!policy) {
			/* first check for parent in the load set */
			p = __list_lookup_parent(&lh, ent->new);
			if (!p) {
				/*
				 * fill in missing parent with null
				 * profile that doesn't have
				 * permissions. This allows for
				 * individual profile loading where
				 * the child is loaded before the
				 * parent, and outside of the current
				 * atomic set. This unfortunately can
				 * happen with some userspaces.  The
				 * null profile will be replaced once
				 * the parent is loaded.
				 */
				policy = __create_missing_ancestors(ns,
							ent->new->base.hname,
							GFP_KERNEL);
				if (!policy) {
					error = -ENOENT;
					info = "parent does not exist";
					goto fail_lock;
				}
			}
		}
		if (!p && policy != &ns->base)
			/* released on profile replacement or free_profile */
			p = (struct aa_profile *) policy;
		rcu_assign_pointer(ent->new->parent, aa_get_profile(p));
	}

	/* create new fs entries for introspection if needed */
	if (!udata->dents[AAFS_LOADDATA_DIR] && aa_g_export_binary) {
		error = __aa_fs_create_rawdata(ns, udata);
		if (error) {
			info = "failed to create raw_data dir and files";
			ent = NULL;
			goto fail_lock;
		}
	}
	list_for_each_entry(ent, &lh, list) {
		if (!ent->old) {
			struct dentry *parent;
			if (rcu_access_pointer(ent->new->parent)) {
				struct aa_profile *p;
				p = aa_deref_parent(ent->new);
				parent = prof_child_dir(p);
			} else
				parent = ns_subprofs_dir(ent->new->ns);
			error = __aafs_profile_mkdir(ent->new, parent);
		}

		if (error) {
			info = "failed to create";
			goto fail_lock;
		}
	}

	/* Done with checks that may fail - do actual replacement */
	__aa_bump_ns_revision(ns);
	if (aa_g_export_binary)
		__aa_loaddata_update(udata, ns->revision);
	list_for_each_entry_safe(ent, tmp, &lh, list) {
		list_del_init(&ent->list);
		op = (!ent->old && !ent->rename) ? OP_PROF_LOAD : OP_PROF_REPL;

		if (ent->old && ent->old->rawdata == ent->new->rawdata &&
		    ent->new->rawdata) {
			/* dedup actual profile replacement */
			audit_policy(label, op, ns_name, ent->new->base.hname,
				     "same as current profile, skipping",
				     error);
			/* break refcount cycle with proxy. */
			aa_put_proxy(ent->new->label.proxy);
			ent->new->label.proxy = NULL;
			goto skip;
		}

		/*
		 * TODO: finer dedup based on profile range in data. Load set
		 * can differ but profile may remain unchanged
		 */
		audit_policy(label, op, ns_name, ent->new->base.hname, NULL,
			     error);

		if (ent->old) {
			share_name(ent->old, ent->new);
			__replace_profile(ent->old, ent->new);
		} else {
			struct list_head *lh;

			if (rcu_access_pointer(ent->new->parent)) {
				struct aa_profile *parent;

				parent = update_to_newest_parent(ent->new);
				lh = &parent->base.profiles;
			} else
				lh = &ns->base.profiles;
			__add_profile(lh, ent->new);
		}
	skip:
		aa_load_ent_free(ent);
	}
	__aa_labelset_update_subtree(ns);
	mutex_unlock(&ns->lock);

out:
	aa_put_ns(ns);
	aa_put_loaddata(udata);
	kfree(ns_name);

	if (error)
		return error;
	return udata->size;

fail_lock:
	mutex_unlock(&ns->lock);

	/* audit cause of failure */
	op = (ent && !ent->old) ? OP_PROF_LOAD : OP_PROF_REPL;
fail:
	  audit_policy(label, op, ns_name, ent ? ent->new->base.hname : NULL,
		       info, error);
	/* audit status that rest of profiles in the atomic set failed too */
	info = "valid profile in failed atomic policy load";
	list_for_each_entry(tmp, &lh, list) {
		if (tmp == ent) {
			info = "unchecked profile in failed atomic policy load";
			/* skip entry that caused failure */
			continue;
		}
		op = (!tmp->old) ? OP_PROF_LOAD : OP_PROF_REPL;
		audit_policy(label, op, ns_name, tmp->new->base.hname, info,
			     error);
	}
	list_for_each_entry_safe(ent, tmp, &lh, list) {
		list_del_init(&ent->list);
		aa_load_ent_free(ent);
	}

	goto out;
}

struct aa_replace_profiles__HEXSHA { void _665b1856dc23; };
struct aa_replace_profiles__ATTRS { };
