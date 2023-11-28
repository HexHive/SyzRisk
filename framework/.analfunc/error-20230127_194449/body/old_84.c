static inline kgid_t
posix_acl_from_xattr_kgid(struct user_namespace *mnt_userns,
			  struct user_namespace *fs_userns,
			  const struct posix_acl_xattr_entry *e)
{
	return make_kgid(fs_userns, le32_to_cpu(e->e_id));
}

struct posix_acl_from_xattr_kgid__HEXSHA { void _0a26bde2c9db; };
struct posix_acl_from_xattr_kgid__ATTRS { };
