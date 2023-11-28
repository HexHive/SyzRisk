void create_servers(struct __test_metadata *_metadata,
		    create_servers(so_incoming_cpu) *self,
		    const  *variant)
{
	int i, ret;

	for (i = 0; i < NR_SERVER; i++) {
		self->servers[i] = create_server(_metadata, self, variant, i);

		if (i == 0) {
			ret = getsockname(self->servers[i], &self->addr, &self->addrlen);
			ASSERT_EQ(ret, 0);
		}
	}

	if (variant->when_to_set == AFTER_ALL_LISTEN) {
		for (i = 0; i < NR_SERVER; i++)
			set_so_incoming_cpu(_metadata, self->servers[i], i);
	}
}

struct create_servers__HEXSHA { void _6df96146b202; };
struct create_servers__ATTRS { void _create_servers; };
