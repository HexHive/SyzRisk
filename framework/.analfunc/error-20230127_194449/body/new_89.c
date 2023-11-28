static int __dwc3_stop_active_transfer(struct dwc3_ep *dep, bool force, bool interrupt)
{
	struct dwc3_gadget_ep_cmd_params params;
	u32 cmd;
	int ret;

	cmd = DWC3_DEPCMD_ENDTRANSFER;
	cmd |= force ? DWC3_DEPCMD_HIPRI_FORCERM : 0;
	cmd |= interrupt ? DWC3_DEPCMD_CMDIOC : 0;
	cmd |= DWC3_DEPCMD_PARAM(dep->resource_index);
	memset(&params, 0, sizeof(params));
	ret = dwc3_send_gadget_ep_cmd(dep, cmd, &params);
	/*
	 * If the End Transfer command was timed out while the device is
	 * not in SETUP phase, it's possible that an incoming Setup packet
	 * may prevent the command's completion. Let's retry when the
	 * ep0state returns to EP0_SETUP_PHASE.
	 */
	if (ret == -ETIMEDOUT && dep->dwc->ep0state != EP0_SETUP_PHASE) {
		dep->flags |= DWC3_EP_DELAY_STOP;
		return 0;
	}
	WARN_ON_ONCE(ret);
	dep->resource_index = 0;

	if (!interrupt)
		dep->flags &= ~DWC3_EP_TRANSFER_STARTED;
	else if (!ret)
		dep->flags |= DWC3_EP_END_TRANSFER_PENDING;

	return ret;
}

struct __dwc3_stop_active_transfer__HEXSHA { void _4db0fbb60136; };
struct __dwc3_stop_active_transfer__ATTRS { };
