void dwc3_stop_active_transfer(struct dwc3_ep *dep, bool force,
	bool interrupt)
{
	struct dwc3 *dwc = dep->dwc;

	/*
	 * Only issue End Transfer command to the control endpoint of a started
	 * Data Phase. Typically we should only do so in error cases such as
	 * invalid/unexpected direction as described in the control transfer
	 * flow of the programming guide.
	 */
	if (dep->number <= 1 && dwc->ep0state != EP0_DATA_PHASE)
		return;

	if (!(dep->flags & DWC3_EP_TRANSFER_STARTED) ||
	    (dep->flags & DWC3_EP_DELAY_STOP) ||
	    (dep->flags & DWC3_EP_END_TRANSFER_PENDING))
		return;

	/*
	 * If a Setup packet is received but yet to DMA out, the controller will
	 * not process the End Transfer command of any endpoint. Polling of its
	 * DEPCMD.CmdAct may block setting up TRB for Setup packet, causing a
	 * timeout. Delay issuing the End Transfer command until the Setup TRB is
	 * prepared.
	 */
	if (dwc->ep0state != EP0_SETUP_PHASE) {
		dep->flags |= DWC3_EP_DELAY_STOP;
		return;
	}

	/*
	 * NOTICE: We are violating what the Databook says about the
	 * EndTransfer command. Ideally we would _always_ wait for the
	 * EndTransfer Command Completion IRQ, but that's causing too
	 * much trouble synchronizing between us and gadget driver.
	 *
	 * We have discussed this with the IP Provider and it was
	 * suggested to giveback all requests here.
	 *
	 * Note also that a similar handling was tested by Synopsys
	 * (thanks a lot Paul) and nothing bad has come out of it.
	 * In short, what we're doing is issuing EndTransfer with
	 * CMDIOC bit set and delay kicking transfer until the
	 * EndTransfer command had completed.
	 *
	 * As of IP version 3.10a of the DWC_usb3 IP, the controller
	 * supports a mode to work around the above limitation. The
	 * software can poll the CMDACT bit in the DEPCMD register
	 * after issuing a EndTransfer command. This mode is enabled
	 * by writing GUCTL2[14]. This polling is already done in the
	 * dwc3_send_gadget_ep_cmd() function so if the mode is
	 * enabled, the EndTransfer command will have completed upon
	 * returning from this function.
	 *
	 * This mode is NOT available on the DWC_usb31 IP.
	 */

	__dwc3_stop_active_transfer(dep, force, interrupt);
}

struct dwc3_stop_active_transfer__HEXSHA { void _4db0fbb60136; };
struct dwc3_stop_active_transfer__ATTRS { };
