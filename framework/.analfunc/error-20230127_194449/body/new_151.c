int mlx5e_xsk_alloc_rx_mpwqe(struct mlx5e_rq *rq, u16 ix)
{
	struct mlx5e_mpw_info *wi = mlx5e_get_mpw_info(rq, ix);
	struct mlx5e_icosq *icosq = rq->icosq;
	struct mlx5_wq_cyc *wq = &icosq->wq;
	struct mlx5e_umr_wqe *umr_wqe;
	int batch, i;
	u32 offset; /* 17-bit value with MTT. */
	u16 pi;

	if (unlikely(!xsk_buff_can_alloc(rq->xsk_pool, rq->mpwqe.pages_per_wqe)))
		goto err;

	BUILD_BUG_ON(sizeof(wi->alloc_units[0]) != sizeof(wi->alloc_units[0].xsk));
	batch = xsk_buff_alloc_batch(rq->xsk_pool, (struct xdp_buff **)wi->alloc_units,
				     rq->mpwqe.pages_per_wqe);

	/* If batch < pages_per_wqe, either:
	 * 1. Some (or all) descriptors were invalid.
	 * 2. dma_need_sync is true, and it fell back to allocating one frame.
	 * In either case, try to continue allocating frames one by one, until
	 * the first error, which will mean there are no more valid descriptors.
	 */
	for (; batch < rq->mpwqe.pages_per_wqe; batch++) {
		wi->alloc_units[batch].xsk = xsk_buff_alloc(rq->xsk_pool);
		if (unlikely(!wi->alloc_units[batch].xsk))
			goto err_reuse_batch;
	}

	pi = mlx5e_icosq_get_next_pi(icosq, rq->mpwqe.umr_wqebbs);
	umr_wqe = mlx5_wq_cyc_get_wqe(wq, pi);
	memcpy(umr_wqe, &rq->mpwqe.umr_wqe, sizeof(struct mlx5e_umr_wqe));

	if (likely(rq->mpwqe.umr_mode == MLX5E_MPWRQ_UMR_MODE_ALIGNED)) {
		for (i = 0; i < batch; i++) {
			dma_addr_t addr = xsk_buff_xdp_get_frame_dma(wi->alloc_units[i].xsk);

			umr_wqe->inline_mtts[i] = (struct mlx5_mtt) {
				.ptag = cpu_to_be64(addr | MLX5_EN_WR),
			};
		}
	} else if (unlikely(rq->mpwqe.umr_mode == MLX5E_MPWRQ_UMR_MODE_UNALIGNED)) {
		for (i = 0; i < batch; i++) {
			dma_addr_t addr = xsk_buff_xdp_get_frame_dma(wi->alloc_units[i].xsk);

			umr_wqe->inline_ksms[i] = (struct mlx5_ksm) {
				.key = rq->mkey_be,
				.va = cpu_to_be64(addr),
			};
		}
	} else if (likely(rq->mpwqe.umr_mode == MLX5E_MPWRQ_UMR_MODE_TRIPLE)) {
		u32 mapping_size = 1 << (rq->mpwqe.page_shift - 2);

		for (i = 0; i < batch; i++) {
			dma_addr_t addr = xsk_buff_xdp_get_frame_dma(wi->alloc_units[i].xsk);

			umr_wqe->inline_ksms[i << 2] = (struct mlx5_ksm) {
				.key = rq->mkey_be,
				.va = cpu_to_be64(addr),
			};
			umr_wqe->inline_ksms[(i << 2) + 1] = (struct mlx5_ksm) {
				.key = rq->mkey_be,
				.va = cpu_to_be64(addr + mapping_size),
			};
			umr_wqe->inline_ksms[(i << 2) + 2] = (struct mlx5_ksm) {
				.key = rq->mkey_be,
				.va = cpu_to_be64(addr + mapping_size * 2),
			};
			umr_wqe->inline_ksms[(i << 2) + 3] = (struct mlx5_ksm) {
				.key = rq->mkey_be,
				.va = cpu_to_be64(rq->wqe_overflow.addr),
			};
		}
	} else {
		__be32 pad_size = cpu_to_be32((1 << rq->mpwqe.page_shift) -
					      rq->xsk_pool->chunk_size);
		__be32 frame_size = cpu_to_be32(rq->xsk_pool->chunk_size);

		for (i = 0; i < batch; i++) {
			dma_addr_t addr = xsk_buff_xdp_get_frame_dma(wi->alloc_units[i].xsk);

			umr_wqe->inline_klms[i << 1] = (struct mlx5_klm) {
				.key = rq->mkey_be,
				.va = cpu_to_be64(addr),
				.bcount = frame_size,
			};
			umr_wqe->inline_klms[(i << 1) + 1] = (struct mlx5_klm) {
				.key = rq->mkey_be,
				.va = cpu_to_be64(rq->wqe_overflow.addr),
				.bcount = pad_size,
			};
		}
	}

	bitmap_zero(wi->xdp_xmit_bitmap, rq->mpwqe.pages_per_wqe);
	wi->consumed_strides = 0;

	umr_wqe->ctrl.opmod_idx_opcode =
		cpu_to_be32((icosq->pc << MLX5_WQE_CTRL_WQE_INDEX_SHIFT) | MLX5_OPCODE_UMR);

	/* Optimized for speed: keep in sync with mlx5e_mpwrq_umr_entry_size. */
	offset = ix * rq->mpwqe.mtts_per_wqe;
	if (likely(rq->mpwqe.umr_mode == MLX5E_MPWRQ_UMR_MODE_ALIGNED))
		offset = offset * sizeof(struct mlx5_mtt) / MLX5_OCTWORD;
	else if (unlikely(rq->mpwqe.umr_mode == MLX5E_MPWRQ_UMR_MODE_OVERSIZED))
		offset = offset * sizeof(struct mlx5_klm) * 2 / MLX5_OCTWORD;
	else if (unlikely(rq->mpwqe.umr_mode == MLX5E_MPWRQ_UMR_MODE_TRIPLE))
		offset = offset * sizeof(struct mlx5_ksm) * 4 / MLX5_OCTWORD;
	umr_wqe->uctrl.xlt_offset = cpu_to_be16(offset);

	icosq->db.wqe_info[pi] = (struct mlx5e_icosq_wqe_info) {
		.wqe_type = MLX5E_ICOSQ_WQE_UMR_RX,
		.num_wqebbs = rq->mpwqe.umr_wqebbs,
		.umr.rq = rq,
	};

	icosq->pc += rq->mpwqe.umr_wqebbs;

	icosq->doorbell_cseg = &umr_wqe->ctrl;

	return 0;

err_reuse_batch:
	while (--batch >= 0)
		xsk_buff_free(wi->alloc_units[batch].xsk);

err:
	rq->stats->buff_alloc_err++;
	return -ENOMEM;
}

struct mlx5e_xsk_alloc_rx_mpwqe__HEXSHA { void _c2c9e31dfa4f; };
struct mlx5e_xsk_alloc_rx_mpwqe__ATTRS { };
