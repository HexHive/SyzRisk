void posix_acl_getxattr_idmapped_mnt(struct user_namespace *mnt_userns,
				     const struct inode *inode,
				     void *value, size_t size)
{
	struct posix_acl_xattr_header *header = value;
	struct posix_acl_xattr_entry *entry = (void *)(header + 1), *end;
	struct user_namespace *fs_userns = i_user_ns(inode);
	int count;
	vfsuid_t vfsuid;
	vfsgid_t vfsgid;
	kuid_t uid;
	kgid_t gid;

	if (no_idmapping(mnt_userns, i_user_ns(inode)))
		return;

	count = posix_acl_fix_xattr_common(value, size);
	if (count <= 0)
		return;

	for (end = entry + count; entry != end; entry++) {
		switch (le16_to_cpu(entry->e_tag)) {
		case ACL_USER:
			uid = make_kuid(&init_user_ns, le32_to_cpu(entry->e_id));
			vfsuid = make_vfsuid(mnt_userns, fs_userns, uid);
			entry->e_id = cpu_to_le32(from_kuid(&init_user_ns,
						vfsuid_into_kuid(vfsuid)));
			break;
		case ACL_GROUP:
			gid = make_kgid(&init_user_ns, le32_to_cpu(entry->e_id));
			vfsgid = make_vfsgid(mnt_userns, fs_userns, gid);
			entry->e_id = cpu_to_le32(from_kgid(&init_user_ns,
						vfsgid_into_kgid(vfsgid)));
			break;
		default:
			break;
		}
	}
}

struct posix_acl_getxattr_idmapped_mnt__HEXSHA { void _0a26bde2c9db; };
struct posix_acl_getxattr_idmapped_mnt__ATTRS { };
