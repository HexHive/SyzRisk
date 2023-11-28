static u8 mlx5e_mpwrq_access_mode(enum mlx5e_mpwrq_umr_mode umr_mode)
{
	switch (umr_mode) {
	case MLX5E_MPWRQ_UMR_MODE_ALIGNED:
		return MLX5_MKC_ACCESS_MODE_MTT;
	case MLX5E_MPWRQ_UMR_MODE_UNALIGNED:
		return MLX5_MKC_ACCESS_MODE_KSM;
	case MLX5E_MPWRQ_UMR_MODE_OVERSIZED:
		return MLX5_MKC_ACCESS_MODE_KLMS;
	case MLX5E_MPWRQ_UMR_MODE_TRIPLE:
		return MLX5_MKC_ACCESS_MODE_KSM;
	}
	WARN_ONCE(1, "MPWRQ UMR mode %d is not known\n", umr_mode);
	return 0;
}

struct mlx5e_mpwrq_access_mode__HEXSHA { void _c2c9e31dfa4f; };
struct mlx5e_mpwrq_access_mode__ATTRS { };
