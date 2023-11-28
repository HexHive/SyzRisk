static u8 mlx5e_build_icosq_log_wq_sz(struct mlx5_core_dev *mdev,
				      struct mlx5e_params *params,
				      struct mlx5e_rq_param *rqp)
{
	u32 wqebbs, total_pages, useful_space;

	/* MLX5_WQ_TYPE_CYCLIC */
	if (params->rq_wq_type != MLX5_WQ_TYPE_LINKED_LIST_STRIDING_RQ)
		return MLX5E_PARAMS_MINIMUM_LOG_SQ_SIZE;

	/* UMR WQEs for the regular RQ. */
	wqebbs = mlx5e_mpwrq_total_umr_wqebbs(mdev, params, NULL);

	/* If XDP program is attached, XSK may be turned on at any time without
	 * restarting the channel. ICOSQ must be big enough to fit UMR WQEs of
	 * both regular RQ and XSK RQ.
	 *
	 * XSK uses different values of page_shift, and the total number of UMR
	 * WQEBBs depends on it. This dependency is complex and not monotonic,
	 * especially taking into consideration that some of the parameters come
	 * from capabilities. Hence, we have to try all valid values of XSK
	 * frame size (and page_shift) to find the maximum.
	 */
	if (params->xdp_prog) {
		u32 max_xsk_wqebbs = 0;
		u8 frame_shift;

		for (frame_shift = XDP_UMEM_MIN_CHUNK_SHIFT;
		     frame_shift <= PAGE_SHIFT; frame_shift++) {
			/* The headroom doesn't affect the calculation. */
			struct mlx5e_xsk_param xsk = {
				.chunk_size = 1 << frame_shift,
				.unaligned = false,
			};

			/* XSK aligned mode. */
			max_xsk_wqebbs = max(max_xsk_wqebbs,
				mlx5e_mpwrq_total_umr_wqebbs(mdev, params, &xsk));

			/* XSK unaligned mode, frame size is a power of two. */
			xsk.unaligned = true;
			max_xsk_wqebbs = max(max_xsk_wqebbs,
				mlx5e_mpwrq_total_umr_wqebbs(mdev, params, &xsk));

			/* XSK unaligned mode, frame size is not equal to stride size. */
			xsk.chunk_size -= 1;
			max_xsk_wqebbs = max(max_xsk_wqebbs,
				mlx5e_mpwrq_total_umr_wqebbs(mdev, params, &xsk));

			/* XSK unaligned mode, frame size is a triple power of two. */
			xsk.chunk_size = (1 << frame_shift) / 4 * 3;
			max_xsk_wqebbs = max(max_xsk_wqebbs,
				mlx5e_mpwrq_total_umr_wqebbs(mdev, params, &xsk));
		}

		wqebbs += max_xsk_wqebbs;
	}

	if (params->packet_merge.type == MLX5E_PACKET_MERGE_SHAMPO)
		wqebbs += mlx5e_shampo_icosq_sz(mdev, params, rqp);

	/* UMR WQEs don't cross the page boundary, they are padded with NOPs.
	 * This padding is always smaller than the max WQE size. That gives us
	 * at least (PAGE_SIZE - (max WQE size - MLX5_SEND_WQE_BB)) useful bytes
	 * per page. The number of pages is estimated as the total size of WQEs
	 * divided by the useful space in page, rounding up. If some WQEs don't
	 * fully fit into the useful space, they can occupy part of the padding,
	 * which proves this estimation to be correct (reserve enough space).
	 */
	useful_space = PAGE_SIZE - mlx5e_get_max_sq_wqebbs(mdev) + MLX5_SEND_WQE_BB;
	total_pages = DIV_ROUND_UP(wqebbs * MLX5_SEND_WQE_BB, useful_space);
	wqebbs = total_pages * (PAGE_SIZE / MLX5_SEND_WQE_BB);

	return max_t(u8, MLX5E_PARAMS_MINIMUM_LOG_SQ_SIZE, order_base_2(wqebbs));
}

struct mlx5e_build_icosq_log_wq_sz__HEXSHA { void _c2c9e31dfa4f; };
struct mlx5e_build_icosq_log_wq_sz__ATTRS { };
