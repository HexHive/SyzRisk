static int
__lcs_start_xmit(struct lcs_card *card, struct sk_buff *skb,
		 struct net_device *dev)
{
	struct lcs_header *header;
	int rc = NETDEV_TX_OK;

	LCS_DBF_TEXT(5, trace, "hardxmit");
	if (skb == NULL) {
		card->stats.tx_dropped++;
		card->stats.tx_errors++;
		return NETDEV_TX_OK;
	}
	if (card->state != DEV_STATE_UP) {
		dev_kfree_skb(skb);
		card->stats.tx_dropped++;
		card->stats.tx_errors++;
		card->stats.tx_carrier_errors++;
		return NETDEV_TX_OK;
	}
	if (skb->protocol == htons(ETH_P_IPV6)) {
		dev_kfree_skb(skb);
		return NETDEV_TX_OK;
	}
	netif_stop_queue(card->dev);
	spin_lock(&card->lock);
	if (card->tx_buffer != NULL &&
	    card->tx_buffer->count + sizeof(struct lcs_header) +
	    skb->len + sizeof(u16) > LCS_IOBUFFERSIZE)
		/* skb too big for current tx buffer. */
		__lcs_emit_txbuffer(card);
	if (card->tx_buffer == NULL) {
		/* Get new tx buffer */
		card->tx_buffer = lcs_get_buffer(&card->write);
		if (card->tx_buffer == NULL) {
			card->stats.tx_dropped++;
			rc = NETDEV_TX_BUSY;
			goto out;
		}
		card->tx_buffer->callback = lcs_txbuffer_cb;
		card->tx_buffer->count = 0;
	}
	header = (struct lcs_header *)
		(card->tx_buffer->data + card->tx_buffer->count);
	card->tx_buffer->count += skb->len + sizeof(struct lcs_header);
	header->offset = card->tx_buffer->count;
	header->type = card->lan_type;
	header->slot = card->portno;
	skb_copy_from_linear_data(skb, header + 1, skb->len);
	spin_unlock(&card->lock);
	card->stats.tx_bytes += skb->len;
	card->stats.tx_packets++;
	dev_kfree_skb(skb);
	netif_wake_queue(card->dev);
	spin_lock(&card->lock);
	if (card->tx_emitted <= 0 && card->tx_buffer != NULL)
		/* If this is the first tx buffer emit it immediately. */
		__lcs_emit_txbuffer(card);
out:
	spin_unlock(&card->lock);
	return rc;
}

struct __lcs_start_xmit__HEXSHA { void _bb16db839365; };
struct __lcs_start_xmit__ATTRS { };
