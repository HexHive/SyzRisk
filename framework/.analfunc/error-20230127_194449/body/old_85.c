void posix_acl_fix_xattr_to_user(void *value, size_t size)
{
	struct user_namespace *user_ns = current_user_ns();
	if (user_ns == &init_user_ns)
		return;
	posix_acl_fix_xattr_userns(user_ns, &init_user_ns, value, size);
}

struct posix_acl_fix_xattr_to_user__HEXSHA { void _0a26bde2c9db; };
struct posix_acl_fix_xattr_to_user__ATTRS { };
