static void brcmstb_reset(struct sdhci_host *host, u8 mask)
{
	struct sdhci_pltfm_host *pltfm_host = sdhci_priv(host);
	struct sdhci_brcmstb_priv *priv = sdhci_pltfm_priv(pltfm_host);

	sdhci_reset(host, mask);

	/* Reset will clear this, so re-enable it */
	if (priv->flags & BRCMSTB_PRIV_FLAGS_GATE_CLOCK)
		enable_clock_gating(host);
}

struct brcmstb_reset__HEXSHA { void _56baa208f910; };
struct brcmstb_reset__ATTRS { };
