ssize_t
vfs_getxattr(struct user_namespace *mnt_userns, struct dentry *dentry,
	     const char *name, void *value, size_t size)
{
	struct inode *inode = dentry->d_inode;
	int error;

	error = xattr_permission(mnt_userns, inode, name, MAY_READ);
	if (error)
		return error;

	error = security_inode_getxattr(dentry, name);
	if (error)
		return error;

	if (!strncmp(name, XATTR_SECURITY_PREFIX,
				XATTR_SECURITY_PREFIX_LEN)) {
		const char *suffix = name + XATTR_SECURITY_PREFIX_LEN;
		int ret = xattr_getsecurity(mnt_userns, inode, suffix, value,
					    size);
		/*
		 * Only overwrite the return value if a security module
		 * is actually active.
		 */
		if (ret == -EOPNOTSUPP)
			goto nolsm;
		return ret;
	}
nolsm:
	return __vfs_getxattr(dentry, inode, name, value, size);
}

struct vfs_getxattr__HEXSHA { void _0a26bde2c9db; };
struct vfs_getxattr__ATTRS { };
