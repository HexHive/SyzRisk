static void ofdpa_port_fdb_learn_work(struct work_struct *work)
{
	const struct ofdpa_fdb_learn_work *lw =
		container_of(work, struct ofdpa_fdb_learn_work, work);
	bool removing = (lw->flags & OFDPA_OP_FLAG_REMOVE);
	struct switchdev_notifier_fdb_info info = {};
	enum switchdev_notifier_type event;

	info.addr = lw->addr;
	info.vid = lw->vid;
	event = removing ? SWITCHDEV_FDB_DEL_TO_BRIDGE :
			   SWITCHDEV_FDB_ADD_TO_BRIDGE;

	rtnl_lock();
	call_switchdev_notifiers(event, lw->ofdpa_port->dev, &info.info, NULL);
	rtnl_unlock();

	kfree(work);
}

struct ofdpa_port_fdb_learn_work__HEXSHA { void _42e51de97cb4; };
struct ofdpa_port_fdb_learn_work__ATTRS { };
