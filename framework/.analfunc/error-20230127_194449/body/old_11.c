static int rk_crypto_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rk_crypto_info *crypto_info;
	int err = 0;

	crypto_info = devm_kzalloc(&pdev->dev,
				   sizeof(*crypto_info), GFP_KERNEL);
	if (!crypto_info) {
		err = -ENOMEM;
		goto err_crypto;
	}

	crypto_info->rst = devm_reset_control_get(dev, "crypto-rst");
	if (IS_ERR(crypto_info->rst)) {
		err = PTR_ERR(crypto_info->rst);
		goto err_crypto;
	}

	reset_control_assert(crypto_info->rst);
	usleep_range(10, 20);
	reset_control_deassert(crypto_info->rst);

	crypto_info->reg = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(crypto_info->reg)) {
		err = PTR_ERR(crypto_info->reg);
		goto err_crypto;
	}

	crypto_info->num_clks = devm_clk_bulk_get_all(&pdev->dev,
						      &crypto_info->clks);
	if (crypto_info->num_clks < 3) {
		err = -EINVAL;
		goto err_crypto;
	}

	crypto_info->irq = platform_get_irq(pdev, 0);
	if (crypto_info->irq < 0) {
		dev_err(&pdev->dev, "control Interrupt is not available.\n");
		err = crypto_info->irq;
		goto err_crypto;
	}

	err = devm_request_irq(&pdev->dev, crypto_info->irq,
			       rk_crypto_irq_handle, IRQF_SHARED,
			       "rk-crypto", pdev);

	if (err) {
		dev_err(&pdev->dev, "irq request failed.\n");
		goto err_crypto;
	}

	crypto_info->dev = &pdev->dev;
	platform_set_drvdata(pdev, crypto_info);

	crypto_info->engine = crypto_engine_alloc_init(&pdev->dev, true);
	crypto_engine_start(crypto_info->engine);
	init_completion(&crypto_info->complete);

	err = rk_crypto_pm_init(crypto_info);
	if (err)
		goto err_pm;

	err = rk_crypto_register(crypto_info);
	if (err) {
		dev_err(dev, "err in register alg");
		goto err_register_alg;
	}


	/* Ignore error of debugfs */
	crypto_info->dbgfs_dir = debugfs_create_dir("rk3288_crypto", NULL);
	crypto_info->dbgfs_stats = debugfs_create_file("stats", 0444,
						       crypto_info->dbgfs_dir,
						       crypto_info,
						       &rk_crypto_debugfs_fops);


	dev_info(dev, "Crypto Accelerator successfully registered\n");
	return 0;

err_register_alg:
	rk_crypto_pm_exit(crypto_info);
err_pm:
	crypto_engine_exit(crypto_info->engine);
err_crypto:
	dev_err(dev, "Crypto Accelerator not successfully registered\n");
	return err;
}

struct rk_crypto_probe__HEXSHA { void _e220e6719438; };
struct rk_crypto_probe__ATTRS { };
