void dc_dmub_setup_subvp_dmub_command(struct dc *dc,
		struct dc_state *context,
		bool enable)
{
	uint8_t cmd_pipe_index = 0;
	uint32_t i, pipe_idx;
	uint8_t subvp_count = 0;
	union dmub_rb_cmd cmd;
	struct pipe_ctx *subvp_pipes[2];
	uint32_t wm_val_refclk = 0;

	memset(&cmd, 0, sizeof(cmd));
	// FW command for SUBVP
	cmd.fw_assisted_mclk_switch_v2.header.type = DMUB_CMD__FW_ASSISTED_MCLK_SWITCH;
	cmd.fw_assisted_mclk_switch_v2.header.sub_type = DMUB_CMD__HANDLE_SUBVP_CMD;
	cmd.fw_assisted_mclk_switch_v2.header.payload_bytes =
			sizeof(cmd.fw_assisted_mclk_switch_v2) - sizeof(cmd.fw_assisted_mclk_switch_v2.header);

	for (i = 0; i < dc->res_pool->pipe_count; i++) {
		struct pipe_ctx *pipe = &context->res_ctx.pipe_ctx[i];

		if (!pipe->stream)
			continue;

		/* For SubVP pipe count, only count the top most (ODM / MPC) pipe
		 */
		if (pipe->plane_state && !pipe->top_pipe && !pipe->prev_odm_pipe &&
				pipe->stream->mall_stream_config.type == SUBVP_MAIN)
			subvp_pipes[subvp_count++] = pipe;
	}

	if (enable) {
		// For each pipe that is a "main" SUBVP pipe, fill in pipe data for DMUB SUBVP cmd
		for (i = 0, pipe_idx = 0; i < dc->res_pool->pipe_count; i++) {
			struct pipe_ctx *pipe = &context->res_ctx.pipe_ctx[i];

			if (!pipe->stream)
				continue;

			/* When populating subvp cmd info, only pass in the top most (ODM / MPC) pipe.
			 * Any ODM or MPC splits being used in SubVP will be handled internally in
			 * populate_subvp_cmd_pipe_info
			 */
			if (pipe->plane_state && pipe->stream->mall_stream_config.paired_stream &&
					!pipe->top_pipe && !pipe->prev_odm_pipe &&
					pipe->stream->mall_stream_config.type == SUBVP_MAIN) {
				populate_subvp_cmd_pipe_info(dc, context, &cmd, pipe, cmd_pipe_index++);
			} else if (pipe->plane_state && pipe->stream->mall_stream_config.type == SUBVP_NONE) {
				// Don't need to check for ActiveDRAMClockChangeMargin < 0, not valid in cases where
				// we run through DML without calculating "natural" P-state support
				populate_subvp_cmd_vblank_pipe_info(dc, context, &cmd, pipe, cmd_pipe_index++);

			}
			pipe_idx++;
		}
		if (subvp_count == 2) {
			update_subvp_prefetch_end_to_mall_start(dc, context, &cmd, subvp_pipes);
		}
		cmd.fw_assisted_mclk_switch_v2.config_data.pstate_allow_width_us = dc->caps.subvp_pstate_allow_width_us;
		cmd.fw_assisted_mclk_switch_v2.config_data.vertical_int_margin_us = dc->caps.subvp_vertical_int_margin_us;

		// Store the original watermark value for this SubVP config so we can lower it when the
		// MCLK switch starts
		wm_val_refclk = context->bw_ctx.bw.dcn.watermarks.a.cstate_pstate.pstate_change_ns *
				(dc->res_pool->ref_clocks.dchub_ref_clock_inKhz / 1000) / 1000;

		cmd.fw_assisted_mclk_switch_v2.config_data.watermark_a_cache = wm_val_refclk < 0xFFFF ? wm_val_refclk : 0xFFFF;
	}
	dc_dmub_srv_cmd_queue(dc->ctx->dmub_srv, &cmd);
	dc_dmub_srv_cmd_execute(dc->ctx->dmub_srv);
	dc_dmub_srv_wait_idle(dc->ctx->dmub_srv);
}

struct dc_dmub_setup_subvp_dmub_command__HEXSHA { void _9799702360d5; };
struct dc_dmub_setup_subvp_dmub_command__ATTRS { };
