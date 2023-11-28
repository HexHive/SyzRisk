static int
posix_acl_xattr_set(const struct xattr_handler *handler,
			   struct user_namespace *mnt_userns,
			   struct dentry *dentry, struct inode *inode,
			   const char *name, const void *value, size_t size,
			   int flags)
{
	struct posix_acl *acl = NULL;
	int ret;

	if (value) {
		/*
		 * By the time we end up here the {g,u}ids stored in
		 * ACL_{GROUP,USER} have already been mapped according to the
		 * caller's idmapping. The vfs_set_acl_prepare() helper will
		 * recover them and take idmapped mounts into account. The
		 * filesystem will receive the POSIX ACLs in the correct
		 * format ready to be cached or written to the backing store
		 * taking the filesystem idmapping into account.
		 */
		acl = vfs_set_acl_prepare(mnt_userns, i_user_ns(inode),
					  value, size);
		if (IS_ERR(acl))
			return PTR_ERR(acl);
	}
	ret = set_posix_acl(mnt_userns, dentry, handler->flags, acl);
	posix_acl_release(acl);
	return ret;
}

struct posix_acl_xattr_set__HEXSHA { void _0a26bde2c9db; };
struct posix_acl_xattr_set__ATTRS { };
