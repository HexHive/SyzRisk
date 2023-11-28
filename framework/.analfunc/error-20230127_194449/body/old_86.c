static inline kgid_t
vfs_set_acl_prepare_kgid(struct user_namespace *mnt_userns,
			 struct user_namespace *fs_userns,
			 const struct posix_acl_xattr_entry *e)
{
	kgid_t kgid = KGIDT_INIT(le32_to_cpu(e->e_id));
	return from_vfsgid(mnt_userns, fs_userns, VFSGIDT_INIT(kgid));
}

struct vfs_set_acl_prepare_kgid__HEXSHA { void _0a26bde2c9db; };
struct vfs_set_acl_prepare_kgid__ATTRS { };
