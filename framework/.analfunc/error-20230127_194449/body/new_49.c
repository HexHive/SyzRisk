static void ath9k_hif_usb_reg_in_cb(struct urb *urb)
{
	struct rx_buf *rx_buf = (struct rx_buf *)urb->context;
	struct hif_device_usb *hif_dev = rx_buf->hif_dev;
	struct sk_buff *skb = rx_buf->skb;
	int ret;

	if (!skb)
		return;

	if (!hif_dev)
		goto free_skb;

	switch (urb->status) {
	case 0:
		break;
	case -ENOENT:
	case -ECONNRESET:
	case -ENODEV:
	case -ESHUTDOWN:
		goto free_skb;
	default:
		skb_reset_tail_pointer(skb);
		skb_trim(skb, 0);

		goto resubmit;
	}

	if (likely(urb->actual_length != 0)) {
		skb_put(skb, urb->actual_length);

		/*
		 * Process the command first.
		 * skb is either freed here or passed to be
		 * managed to another callback function.
		 */
		ath9k_htc_rx_msg(hif_dev->htc_handle, skb,
				 skb->len, USB_REG_IN_PIPE);

		skb = alloc_skb(MAX_REG_IN_BUF_SIZE, GFP_ATOMIC);
		if (!skb) {
			dev_err(&hif_dev->udev->dev,
				"ath9k_htc: REG_IN memory allocation failure\n");
			goto free_rx_buf;
		}

		rx_buf->skb = skb;

		usb_fill_int_urb(urb, hif_dev->udev,
				 usb_rcvintpipe(hif_dev->udev,
						 USB_REG_IN_PIPE),
				 skb->data, MAX_REG_IN_BUF_SIZE,
				 ath9k_hif_usb_reg_in_cb, rx_buf, 1);
	}

resubmit:
	usb_anchor_urb(urb, &hif_dev->reg_in_submitted);
	ret = usb_submit_urb(urb, GFP_ATOMIC);
	if (ret) {
		usb_unanchor_urb(urb);
		goto free_skb;
	}

	return;
free_skb:
	kfree_skb(skb);
free_rx_buf:
	kfree(rx_buf);
	urb->context = NULL;
}

struct ath9k_hif_usb_reg_in_cb__HEXSHA { void _dd95f2239fc8; };
struct ath9k_hif_usb_reg_in_cb__ATTRS { };
