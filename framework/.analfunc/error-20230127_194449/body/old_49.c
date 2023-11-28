static void ofdpa_port_fdb_learn_work(struct work_struct *work)
{
	const struct ofdpa_fdb_learn_work *lw =
		container_of(work, struct ofdpa_fdb_learn_work, work);
	bool removing = (lw->flags & OFDPA_OP_FLAG_REMOVE);
	bool learned = (lw->flags & OFDPA_OP_FLAG_LEARNED);
	struct switchdev_notifier_fdb_info info = {};

	info.addr = lw->addr;
	info.vid = lw->vid;

	rtnl_lock();
	if (learned && removing)
		call_switchdev_notifiers(SWITCHDEV_FDB_DEL_TO_BRIDGE,
					 lw->ofdpa_port->dev, &info.info, NULL);
	else if (learned && !removing)
		call_switchdev_notifiers(SWITCHDEV_FDB_ADD_TO_BRIDGE,
					 lw->ofdpa_port->dev, &info.info, NULL);
	rtnl_unlock();

	kfree(work);
}

struct ofdpa_port_fdb_learn_work__HEXSHA { void _42e51de97cb4; };
struct ofdpa_port_fdb_learn_work__ATTRS { };
