static netdev_tx_t lcs_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct lcs_card *card;
	int rc;

	LCS_DBF_TEXT(5, trace, "pktxmit");
	card = (struct lcs_card *) dev->ml_priv;
	rc = __lcs_start_xmit(card, skb, dev);
	return rc;
}

struct lcs_start_xmit__HEXSHA { void _bb16db839365; };
struct lcs_start_xmit__ATTRS { };
