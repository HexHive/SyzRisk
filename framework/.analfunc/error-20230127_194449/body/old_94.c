static struct posix_acl *posix_acl_from_xattr(struct user_namespace *mnt_userns,
	struct user_namespace *fs_userns, const void *value, size_t size,
	kuid_t (*uid_cb)(struct user_namespace *, struct user_namespace *,
			 const struct posix_acl_xattr_entry *),
	kgid_t (*gid_cb)(struct user_namespace *, struct user_namespace *,
			 const struct posix_acl_xattr_entry *))
{
	const struct posix_acl_xattr_header *header = value;
	const struct posix_acl_xattr_entry *entry = (const void *)(header + 1), *end;
	int count;
	struct posix_acl *acl;
	struct posix_acl_entry *acl_e;

	count = posix_acl_fix_xattr_common(value, size);
	if (count < 0)
		return ERR_PTR(count);
	if (count == 0)
		return NULL;
	
	acl = posix_acl_alloc(count, GFP_NOFS);
	if (!acl)
		return ERR_PTR(-ENOMEM);
	acl_e = acl->a_entries;
	
	for (end = entry + count; entry != end; acl_e++, entry++) {
		acl_e->e_tag  = le16_to_cpu(entry->e_tag);
		acl_e->e_perm = le16_to_cpu(entry->e_perm);

		switch(acl_e->e_tag) {
			case ACL_USER_OBJ:
			case ACL_GROUP_OBJ:
			case ACL_MASK:
			case ACL_OTHER:
				break;

			case ACL_USER:
				acl_e->e_uid = uid_cb(mnt_userns, fs_userns, entry);
				if (!uid_valid(acl_e->e_uid))
					goto fail;
				break;
			case ACL_GROUP:
				acl_e->e_gid = gid_cb(mnt_userns, fs_userns, entry);
				if (!gid_valid(acl_e->e_gid))
					goto fail;
				break;

			default:
				goto fail;
		}
	}
	return acl;

fail:
	posix_acl_release(acl);
	return ERR_PTR(-EINVAL);
}

struct posix_acl_from_xattr__HEXSHA { void _0a26bde2c9db; };
struct posix_acl_from_xattr__ATTRS { };
