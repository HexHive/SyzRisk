static int ofdpa_port_fdb_learn(struct ofdpa_port *ofdpa_port,
				int flags, const u8 *addr, __be16 vlan_id)
{
	struct ofdpa_fdb_learn_work *lw;
	enum rocker_of_dpa_table_id goto_tbl =
			ROCKER_OF_DPA_TABLE_ID_ACL_POLICY;
	u32 out_pport = ofdpa_port->pport;
	u32 tunnel_id = 0;
	u32 group_id = ROCKER_GROUP_NONE;
	bool copy_to_cpu = false;
	int err;

	if (ofdpa_port_is_bridged(ofdpa_port))
		group_id = ROCKER_GROUP_L2_INTERFACE(vlan_id, out_pport);

	if (!(flags & OFDPA_OP_FLAG_REFRESH)) {
		err = ofdpa_flow_tbl_bridge(ofdpa_port, flags, addr,
					    NULL, vlan_id, tunnel_id, goto_tbl,
					    group_id, copy_to_cpu);
		if (err)
			return err;
	}

	if (!ofdpa_port_is_bridged(ofdpa_port))
		return 0;

	if (!(flags & OFDPA_OP_FLAG_LEARNED))
		return 0;

	lw = kzalloc(sizeof(*lw), GFP_ATOMIC);
	if (!lw)
		return -ENOMEM;

	INIT_WORK(&lw->work, ofdpa_port_fdb_learn_work);

	lw->ofdpa_port = ofdpa_port;
	lw->flags = flags;
	ether_addr_copy(lw->addr, addr);
	lw->vid = ofdpa_port_vlan_to_vid(ofdpa_port, vlan_id);

	schedule_work(&lw->work);
	return 0;
}

struct ofdpa_port_fdb_learn__HEXSHA { void _42e51de97cb4; };
struct ofdpa_port_fdb_learn__ATTRS { };
