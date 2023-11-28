u8 mlx5e_mpwrq_umr_entry_size(enum mlx5e_mpwrq_umr_mode mode)
{
	switch (mode) {
	case MLX5E_MPWRQ_UMR_MODE_ALIGNED:
		return sizeof(struct mlx5_mtt);
	case MLX5E_MPWRQ_UMR_MODE_UNALIGNED:
		return sizeof(struct mlx5_ksm);
	case MLX5E_MPWRQ_UMR_MODE_OVERSIZED:
		return sizeof(struct mlx5_klm) * 2;
	case MLX5E_MPWRQ_UMR_MODE_TRIPLE:
		return sizeof(struct mlx5_ksm) * 4;
	}
	WARN_ONCE(1, "MPWRQ UMR mode %d is not known\n", mode);
	return 0;
}

struct mlx5e_mpwrq_umr_entry_size__HEXSHA { void _c2c9e31dfa4f; };
struct mlx5e_mpwrq_umr_entry_size__ATTRS { };
