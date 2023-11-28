static inline void jz4740_i2s_set_bits(const struct jz4740_i2s *i2s,
	unsigned int reg, uint32_t bits)
{
	uint32_t value = jz4740_i2s_read(i2s, reg);
	value |= bits;
	jz4740_i2s_write(i2s, reg, value);
}

struct jz4740_i2s_set_bits__HEXSHA { void _8b3a9ad86239; };
struct jz4740_i2s_set_bits__ATTRS { };
