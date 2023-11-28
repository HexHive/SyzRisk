static int jz4740_i2s_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	struct jz4740_i2s *i2s = snd_soc_dai_get_drvdata(dai);
	uint32_t conf;
	int ret;

	/*
	 * When we can flush FIFOs independently, only flush the FIFO
	 * that is starting up. We can do this when the DAI is active
	 * because it does not disturb other active substreams.
	 */
	if (!i2s->soc_info->shared_fifo_flush) {
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
			jz4740_i2s_set_bits(i2s, JZ_REG_AIC_CTRL, JZ_AIC_CTRL_TFLUSH);
		else
			jz4740_i2s_set_bits(i2s, JZ_REG_AIC_CTRL, JZ_AIC_CTRL_RFLUSH);
	}

	if (snd_soc_dai_active(dai))
		return 0;

	/*
	 * When there is a shared flush bit for both FIFOs, the TFLUSH
	 * bit flushes both FIFOs. Flushing while the DAI is active would
	 * cause FIFO underruns in other active substreams so we have to
	 * guard this behind the snd_soc_dai_active() check.
	 */
	if (i2s->soc_info->shared_fifo_flush)
		jz4740_i2s_set_bits(i2s, JZ_REG_AIC_CTRL, JZ_AIC_CTRL_TFLUSH);

	ret = clk_prepare_enable(i2s->clk_i2s);
	if (ret)
		return ret;

	conf = jz4740_i2s_read(i2s, JZ_REG_AIC_CONF);
	conf |= JZ_AIC_CONF_ENABLE;
	jz4740_i2s_write(i2s, JZ_REG_AIC_CONF, conf);

	return 0;
}

struct jz4740_i2s_startup__HEXSHA { void _8b3a9ad86239; };
struct jz4740_i2s_startup__ATTRS { };
