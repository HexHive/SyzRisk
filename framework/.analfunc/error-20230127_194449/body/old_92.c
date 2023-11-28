struct posix_acl *posix_acl_xattr_name(struct user_namespace *mnt_userns,
				      struct user_namespace *fs_userns,
				      const void *value, size_t size)
{
	return make_posix_acl(mnt_userns, fs_userns, value, size,
			      vfs_set_acl_prepare_kuid,
			      vfs_set_acl_prepare_kgid);
}

struct posix_acl_xattr_name__HEXSHA { void _0a26bde2c9db; };
struct posix_acl_xattr_name__ATTRS { };
