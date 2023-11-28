static inline kuid_t
vfs_set_acl_prepare_kuid(struct user_namespace *mnt_userns,
			 struct user_namespace *fs_userns,
			 const struct posix_acl_xattr_entry *e)
{
	kuid_t kuid = KUIDT_INIT(le32_to_cpu(e->e_id));
	return from_vfsuid(mnt_userns, fs_userns, VFSUIDT_INIT(kuid));
}

struct vfs_set_acl_prepare_kuid__HEXSHA { void _0a26bde2c9db; };
struct vfs_set_acl_prepare_kuid__ATTRS { };
