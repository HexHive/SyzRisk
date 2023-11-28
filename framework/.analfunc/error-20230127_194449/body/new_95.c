static struct aa_policy *__create_missing_ancestors(struct aa_ns *ns,
						    const char *hname,
						    gfp_t gfp)
{
	struct aa_policy *policy;
	struct aa_profile *parent, *profile = NULL;
	char *split;

	AA_BUG(!ns);
	AA_BUG(!hname);

	policy = &ns->base;

	for (split = strstr(hname, "//"); split;) {
		parent = profile;
		profile = __strn_find_child(&policy->profiles, hname,
					    split - hname);
		if (!profile) {
			const char *name = kstrndup(hname, split - hname,
						    gfp);
			if (!name)
				return NULL;
			profile = aa_alloc_null(parent, name, gfp);
			kfree(name);
			if (!profile)
				return NULL;
			if (!parent)
				profile->ns = aa_get_ns(ns);
		}
		policy = &profile->base;
		hname = split + 2;
		split = strstr(hname, "//");
	}

struct __create_missing_ancestors__HEXSHA { void _665b1856dc23; };
struct __create_missing_ancestors__ATTRS { };
