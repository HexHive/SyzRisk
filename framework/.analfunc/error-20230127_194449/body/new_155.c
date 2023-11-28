static int mlx5e_create_umr_mkey(struct mlx5_core_dev *mdev,
				 u32 npages, u8 page_shift, u32 *umr_mkey,
				 dma_addr_t filler_addr,
				 enum mlx5e_mpwrq_umr_mode umr_mode,
				 u32 xsk_chunk_size)
{
	struct mlx5_mtt *mtt;
	struct mlx5_ksm *ksm;
	struct mlx5_klm *klm;
	u32 octwords;
	int inlen;
	void *mkc;
	u32 *in;
	int err;
	int i;

	if ((umr_mode == MLX5E_MPWRQ_UMR_MODE_UNALIGNED ||
	     umr_mode == MLX5E_MPWRQ_UMR_MODE_TRIPLE) &&
	    !MLX5_CAP_GEN(mdev, fixed_buffer_size)) {
		mlx5_core_warn(mdev, "Unaligned AF_XDP requires fixed_buffer_size capability\n");
		return -EINVAL;
	}

	octwords = mlx5e_mpwrq_umr_octowords(npages, umr_mode);

	inlen = MLX5_FLEXIBLE_INLEN(mdev, MLX5_ST_SZ_BYTES(create_mkey_in),
				    MLX5_OCTWORD, octwords);
	if (inlen < 0)
		return inlen;

	in = kvzalloc(inlen, GFP_KERNEL);
	if (!in)
		return -ENOMEM;

	mkc = MLX5_ADDR_OF(create_mkey_in, in, memory_key_mkey_entry);

	MLX5_SET(mkc, mkc, free, 1);
	MLX5_SET(mkc, mkc, umr_en, 1);
	MLX5_SET(mkc, mkc, lw, 1);
	MLX5_SET(mkc, mkc, lr, 1);
	MLX5_SET(mkc, mkc, access_mode_1_0, mlx5e_mpwrq_access_mode(umr_mode));
	mlx5e_mkey_set_relaxed_ordering(mdev, mkc);
	MLX5_SET(mkc, mkc, qpn, 0xffffff);
	MLX5_SET(mkc, mkc, pd, mdev->mlx5e_res.hw_objs.pdn);
	MLX5_SET64(mkc, mkc, len, npages << page_shift);
	MLX5_SET(mkc, mkc, translations_octword_size, octwords);
	if (umr_mode == MLX5E_MPWRQ_UMR_MODE_TRIPLE)
		MLX5_SET(mkc, mkc, log_page_size, page_shift - 2);
	else if (umr_mode != MLX5E_MPWRQ_UMR_MODE_OVERSIZED)
		MLX5_SET(mkc, mkc, log_page_size, page_shift);
	MLX5_SET(create_mkey_in, in, translations_octword_actual_size, octwords);

	/* Initialize the mkey with all MTTs pointing to a default
	 * page (filler_addr). When the channels are activated, UMR
	 * WQEs will redirect the RX WQEs to the actual memory from
	 * the RQ's pool, while the gaps (wqe_overflow) remain mapped
	 * to the default page.
	 */
	switch (umr_mode) {
	case MLX5E_MPWRQ_UMR_MODE_OVERSIZED:
		klm = MLX5_ADDR_OF(create_mkey_in, in, klm_pas_mtt);
		for (i = 0; i < npages; i++) {
			klm[i << 1] = (struct mlx5_klm) {
				.va = cpu_to_be64(filler_addr),
				.bcount = cpu_to_be32(xsk_chunk_size),
				.key = cpu_to_be32(mdev->mlx5e_res.hw_objs.mkey),
			};
			klm[(i << 1) + 1] = (struct mlx5_klm) {
				.va = cpu_to_be64(filler_addr),
				.bcount = cpu_to_be32((1 << page_shift) - xsk_chunk_size),
				.key = cpu_to_be32(mdev->mlx5e_res.hw_objs.mkey),
			};
		}
		break;
	case MLX5E_MPWRQ_UMR_MODE_UNALIGNED:
		ksm = MLX5_ADDR_OF(create_mkey_in, in, klm_pas_mtt);
		for (i = 0; i < npages; i++)
			ksm[i] = (struct mlx5_ksm) {
				.key = cpu_to_be32(mdev->mlx5e_res.hw_objs.mkey),
				.va = cpu_to_be64(filler_addr),
			};
		break;
	case MLX5E_MPWRQ_UMR_MODE_ALIGNED:
		mtt = MLX5_ADDR_OF(create_mkey_in, in, klm_pas_mtt);
		for (i = 0; i < npages; i++)
			mtt[i] = (struct mlx5_mtt) {
				.ptag = cpu_to_be64(filler_addr),
			};
		break;
	case MLX5E_MPWRQ_UMR_MODE_TRIPLE:
		ksm = MLX5_ADDR_OF(create_mkey_in, in, klm_pas_mtt);
		for (i = 0; i < npages * 4; i++) {
			ksm[i] = (struct mlx5_ksm) {
				.key = cpu_to_be32(mdev->mlx5e_res.hw_objs.mkey),
				.va = cpu_to_be64(filler_addr),
			};
		}
		break;
	}

	err = mlx5_core_create_mkey(mdev, umr_mkey, in, inlen);

	kvfree(in);
	return err;
}

struct mlx5e_create_umr_mkey__HEXSHA { void _c2c9e31dfa4f; };
struct mlx5e_create_umr_mkey__ATTRS { };
