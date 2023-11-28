enum mlx5e_mpwrq_umr_mode
mlx5e_mpwrq_umr_mode(struct mlx5_core_dev *mdev, struct mlx5e_xsk_param *xsk)
{
	/* Different memory management schemes use different mechanisms to map
	 * user-mode memory. The stricter guarantees we have, the faster
	 * mechanisms we use:
	 * 1. MTT - direct mapping in page granularity.
	 * 2. KSM - indirect mapping to another MKey to arbitrary addresses, but
	 *    all mappings have the same size.
	 * 3. KLM - indirect mapping to another MKey to arbitrary addresses, and
	 *    mappings can have different sizes.
	 */
	u8 page_shift = mlx5e_mpwrq_page_shift(mdev, xsk);
	bool unaligned = xsk ? xsk->unaligned : false;
	bool oversized = false;

	if (xsk) {
		oversized = xsk->chunk_size < (1 << page_shift);
		WARN_ON_ONCE(xsk->chunk_size > (1 << page_shift));
	}

	/* XSK frame size doesn't match the UMR page size, either because the
	 * frame size is not a power of two, or it's smaller than the minimal
	 * page size supported by the firmware.
	 * It's possible to receive packets bigger than MTU in certain setups.
	 * To avoid writing over the XSK frame boundary, the top region of each
	 * stride is mapped to a garbage page, resulting in two mappings of
	 * different sizes per frame.
	 */
	if (oversized) {
		/* An optimization for frame sizes equal to 3 * power_of_two.
		 * 3 KSMs point to the frame, and one KSM points to the garbage
		 * page, which works faster than KLM.
		 */
		if (xsk->chunk_size % 3 == 0 && is_power_of_2(xsk->chunk_size / 3))
			return MLX5E_MPWRQ_UMR_MODE_TRIPLE;

		return MLX5E_MPWRQ_UMR_MODE_OVERSIZED;
	}

	/* XSK frames can start at arbitrary unaligned locations, but they all
	 * have the same size which is a power of two. It allows to optimize to
	 * one KSM per frame.
	 */
	if (unaligned)
		return MLX5E_MPWRQ_UMR_MODE_UNALIGNED;

	/* XSK: frames are naturally aligned, MTT can be used.
	 * Non-XSK: Allocations happen in units of CPU pages, therefore, the
	 * mappings are naturally aligned.
	 */
	return MLX5E_MPWRQ_UMR_MODE_ALIGNED;
}

struct mlx5e_mpwrq_umr_mode__HEXSHA { void _c2c9e31dfa4f; };
struct mlx5e_mpwrq_umr_mode__ATTRS { };
