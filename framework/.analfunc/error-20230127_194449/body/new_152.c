u32 mlx5e_mpwrq_max_num_entries(struct mlx5_core_dev *mdev,
				enum mlx5e_mpwrq_umr_mode umr_mode)
{
	/* Same limits apply to KSMs and KLMs. */
	u32 klm_limit = min(MLX5E_MAX_RQ_NUM_KSMS,
			    1 << MLX5_CAP_GEN(mdev, log_max_klm_list_size));

	switch (umr_mode) {
	case MLX5E_MPWRQ_UMR_MODE_ALIGNED:
		return MLX5E_MAX_RQ_NUM_MTTS;
	case MLX5E_MPWRQ_UMR_MODE_UNALIGNED:
		return klm_limit;
	case MLX5E_MPWRQ_UMR_MODE_OVERSIZED:
		/* Each entry is two KLMs. */
		return klm_limit / 2;
	case MLX5E_MPWRQ_UMR_MODE_TRIPLE:
		/* Each entry is four KSMs. */
		return klm_limit / 4;
	}
	WARN_ONCE(1, "MPWRQ UMR mode %d is not known\n", umr_mode);
	return 0;
}

struct mlx5e_mpwrq_max_num_entries__HEXSHA { void _c2c9e31dfa4f; };
struct mlx5e_mpwrq_max_num_entries__ATTRS { };
