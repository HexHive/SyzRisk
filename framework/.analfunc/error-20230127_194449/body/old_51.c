static struct gsi_trans *gsi_channel_poll_one(struct gsi_channel *channel)
{
	struct gsi_trans *trans;

	/* Get the first transaction from the completed list */
	trans = gsi_channel_trans_complete(channel);
	if (trans)
		gsi_trans_move_polled(trans);

	return trans;
}

struct gsi_channel_poll_one__HEXSHA { void _ace5dc61620b; };
struct gsi_channel_poll_one__ATTRS { };
