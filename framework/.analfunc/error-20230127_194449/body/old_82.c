static int
posix_acl_xattr_get(const struct xattr_handler *handler,
		    struct dentry *unused, struct inode *inode,
		    const char *name, void *value, size_t size)
{
	struct posix_acl *acl;
	int error;

	if (!IS_POSIXACL(inode))
		return -EOPNOTSUPP;
	if (S_ISLNK(inode->i_mode))
		return -EOPNOTSUPP;

	acl = get_inode_acl(inode, handler->flags);
	if (IS_ERR(acl))
		return PTR_ERR(acl);
	if (acl == NULL)
		return -ENODATA;

	error = posix_acl_to_xattr(&init_user_ns, acl, value, size);
	posix_acl_release(acl);

	return error;
}

struct posix_acl_xattr_get__HEXSHA { void _0a26bde2c9db; };
struct posix_acl_xattr_get__ATTRS { };
