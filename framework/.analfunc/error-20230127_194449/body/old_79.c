struct posix_acl *
posix_acl_from_xattr(struct user_namespace *fs_userns,
		     const void *value, size_t size)
{
	return make_posix_acl(&init_user_ns, fs_userns, value, size,
			      posix_acl_from_xattr_kuid,
			      posix_acl_from_xattr_kgid);
}

struct posix_acl_from_xattr__HEXSHA { void _0a26bde2c9db; };
struct posix_acl_from_xattr__ATTRS { };
