static int tsl2583_probe(struct i2c_client *clientp,
			 const struct i2c_device_id *idp)
{
	int ret;
	struct tsl2583_chip *chip;
	struct iio_dev *indio_dev;

	if (!i2c_check_functionality(clientp->adapter,
				     I2C_FUNC_SMBUS_BYTE_DATA)) {
		dev_err(&clientp->dev, "%s: i2c smbus byte data functionality is unsupported\n",
			__func__);
		return -EOPNOTSUPP;
	}

	indio_dev = devm_iio_device_alloc(&clientp->dev, sizeof(*chip));
	if (!indio_dev)
		return -ENOMEM;

	chip = iio_priv(indio_dev);
	chip->client = clientp;
	i2c_set_clientdata(clientp, indio_dev);

	mutex_init(&chip->als_mutex);

	ret = i2c_smbus_read_byte_data(clientp,
				       TSL2583_CMD_REG | TSL2583_CHIPID);
	if (ret < 0) {
		dev_err(&clientp->dev,
			"%s: failed to read the chip ID register\n", __func__);
		return ret;
	}

	if ((ret & TSL2583_CHIP_ID_MASK) != TSL2583_CHIP_ID) {
		dev_err(&clientp->dev, "%s: received an unknown chip ID %x\n",
			__func__, ret);
		return -EINVAL;
	}

	indio_dev->info = &tsl2583_info;
	indio_dev->channels = tsl2583_channels;
	indio_dev->num_channels = ARRAY_SIZE(tsl2583_channels);
	indio_dev->modes = INDIO_DIRECT_MODE;
	indio_dev->name = chip->client->name;

	pm_runtime_enable(&clientp->dev);
	pm_runtime_set_autosuspend_delay(&clientp->dev,
					 TSL2583_POWER_OFF_DELAY_MS);
	pm_runtime_use_autosuspend(&clientp->dev);

	ret = devm_iio_device_register(indio_dev->dev.parent, indio_dev);
	if (ret) {
		dev_err(&clientp->dev, "%s: iio registration failed\n",
			__func__);
		return ret;
	}

	/* Load up the V2 defaults (these are hard coded defaults for now) */
	tsl2583_defaults(chip);

	dev_info(&clientp->dev, "Light sensor found.\n");

	return 0;
}

struct tsl2583_probe__HEXSHA { void _0dec4d2f2636; };
struct tsl2583_probe__ATTRS { };
