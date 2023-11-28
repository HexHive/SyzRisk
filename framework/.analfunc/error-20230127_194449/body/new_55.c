void intel_device_info_runtime_init(struct drm_i915_private *dev_priv)
{
	struct intel_device_info *info = mkwrite_device_info(dev_priv);
	struct intel_runtime_info *runtime = RUNTIME_INFO(dev_priv);
	enum pipe pipe;

	/* Wa_14011765242: adl-s A0,A1 */
	if (IS_ADLS_DISPLAY_STEP(dev_priv, STEP_A0, STEP_A2))
		for_each_pipe(dev_priv, pipe)
			runtime->num_scalers[pipe] = 0;
	else if (DISPLAY_VER(dev_priv) >= 11) {
		for_each_pipe(dev_priv, pipe)
			runtime->num_scalers[pipe] = 2;
	} else if (DISPLAY_VER(dev_priv) >= 9) {
		runtime->num_scalers[PIPE_A] = 2;
		runtime->num_scalers[PIPE_B] = 2;
		runtime->num_scalers[PIPE_C] = 1;
	}

	BUILD_BUG_ON(BITS_PER_TYPE(intel_engine_mask_t) < I915_NUM_ENGINES);

	if (DISPLAY_VER(dev_priv) >= 13 || HAS_D12_PLANE_MINIMIZATION(dev_priv))
		for_each_pipe(dev_priv, pipe)
			runtime->num_sprites[pipe] = 4;
	else if (DISPLAY_VER(dev_priv) >= 11)
		for_each_pipe(dev_priv, pipe)
			runtime->num_sprites[pipe] = 6;
	else if (DISPLAY_VER(dev_priv) == 10)
		for_each_pipe(dev_priv, pipe)
			runtime->num_sprites[pipe] = 3;
	else if (IS_BROXTON(dev_priv)) {
		/*
		 * Skylake and Broxton currently don't expose the topmost plane as its
		 * use is exclusive with the legacy cursor and we only want to expose
		 * one of those, not both. Until we can safely expose the topmost plane
		 * as a DRM_PLANE_TYPE_CURSOR with all the features exposed/supported,
		 * we don't expose the topmost plane at all to prevent ABI breakage
		 * down the line.
		 */

		runtime->num_sprites[PIPE_A] = 2;
		runtime->num_sprites[PIPE_B] = 2;
		runtime->num_sprites[PIPE_C] = 1;
	} else if (IS_VALLEYVIEW(dev_priv) || IS_CHERRYVIEW(dev_priv)) {
		for_each_pipe(dev_priv, pipe)
			runtime->num_sprites[pipe] = 2;
	} else if (DISPLAY_VER(dev_priv) >= 5 || IS_G4X(dev_priv)) {
		for_each_pipe(dev_priv, pipe)
			runtime->num_sprites[pipe] = 1;
	}

	if (HAS_DISPLAY(dev_priv) && IS_GRAPHICS_VER(dev_priv, 7, 8) &&
	    HAS_PCH_SPLIT(dev_priv)) {
		u32 fuse_strap = intel_de_read(dev_priv, FUSE_STRAP);
		u32 sfuse_strap = intel_de_read(dev_priv, SFUSE_STRAP);

		/*
		 * SFUSE_STRAP is supposed to have a bit signalling the display
		 * is fused off. Unfortunately it seems that, at least in
		 * certain cases, fused off display means that PCH display
		 * reads don't land anywhere. In that case, we read 0s.
		 *
		 * On CPT/PPT, we can detect this case as SFUSE_STRAP_FUSE_LOCK
		 * should be set when taking over after the firmware.
		 */
		if (fuse_strap & ILK_INTERNAL_DISPLAY_DISABLE ||
		    sfuse_strap & SFUSE_STRAP_DISPLAY_DISABLED ||
		    (HAS_PCH_CPT(dev_priv) &&
		     !(sfuse_strap & SFUSE_STRAP_FUSE_LOCK))) {
			drm_info(&dev_priv->drm,
				 "Display fused off, disabling\n");
			runtime->pipe_mask = 0;
			runtime->cpu_transcoder_mask = 0;
			runtime->fbc_mask = 0;
		} else if (fuse_strap & IVB_PIPE_C_DISABLE) {
			drm_info(&dev_priv->drm, "PipeC fused off\n");
			runtime->pipe_mask &= ~BIT(PIPE_C);
			runtime->cpu_transcoder_mask &= ~BIT(TRANSCODER_C);
		}
	} else if (HAS_DISPLAY(dev_priv) && DISPLAY_VER(dev_priv) >= 9) {
		u32 dfsm = intel_de_read(dev_priv, SKL_DFSM);

		if (dfsm & SKL_DFSM_PIPE_A_DISABLE) {
			runtime->pipe_mask &= ~BIT(PIPE_A);
			runtime->cpu_transcoder_mask &= ~BIT(TRANSCODER_A);
			runtime->fbc_mask &= ~BIT(INTEL_FBC_A);
		}
		if (dfsm & SKL_DFSM_PIPE_B_DISABLE) {
			runtime->pipe_mask &= ~BIT(PIPE_B);
			runtime->cpu_transcoder_mask &= ~BIT(TRANSCODER_B);
		}
		if (dfsm & SKL_DFSM_PIPE_C_DISABLE) {
			runtime->pipe_mask &= ~BIT(PIPE_C);
			runtime->cpu_transcoder_mask &= ~BIT(TRANSCODER_C);
		}

		if (DISPLAY_VER(dev_priv) >= 12 &&
		    (dfsm & TGL_DFSM_PIPE_D_DISABLE)) {
			runtime->pipe_mask &= ~BIT(PIPE_D);
			runtime->cpu_transcoder_mask &= ~BIT(TRANSCODER_D);
		}

		if (dfsm & SKL_DFSM_DISPLAY_HDCP_DISABLE)
			runtime->has_hdcp = 0;

		if (dfsm & SKL_DFSM_DISPLAY_PM_DISABLE)
			runtime->fbc_mask = 0;

		if (DISPLAY_VER(dev_priv) >= 11 && (dfsm & ICL_DFSM_DMC_DISABLE))
			runtime->has_dmc = 0;

		if (DISPLAY_VER(dev_priv) >= 10 &&
		    (dfsm & GLK_DFSM_DISPLAY_DSC_DISABLE))
			runtime->has_dsc = 0;
	}

	if (GRAPHICS_VER(dev_priv) == 6 && i915_vtd_active(dev_priv)) {
		drm_info(&dev_priv->drm,
			 "Disabling ppGTT for VT-d support\n");
		runtime->ppgtt_type = INTEL_PPGTT_NONE;
	}

	runtime->rawclk_freq = intel_read_rawclk(dev_priv);
	drm_dbg(&dev_priv->drm, "rawclk rate: %d kHz\n", runtime->rawclk_freq);

	if (!HAS_DISPLAY(dev_priv)) {
		dev_priv->drm.driver_features &= ~(DRIVER_MODESET |
						   DRIVER_ATOMIC);
		memset(&info->display, 0, sizeof(info->display));

		runtime->cpu_transcoder_mask = 0;
		memset(runtime->num_sprites, 0, sizeof(runtime->num_sprites));
		memset(runtime->num_scalers, 0, sizeof(runtime->num_scalers));
		runtime->fbc_mask = 0;
		runtime->has_hdcp = false;
		runtime->has_dmc = false;
		runtime->has_dsc = false;
	}

	/* Disable nuclear pageflip by default on pre-ILK */
	if (!dev_priv->params.nuclear_pageflip && DISPLAY_VER(dev_priv) < 5)
		dev_priv->drm.driver_features &= ~DRIVER_ATOMIC;
}

struct intel_device_info_runtime_init__HEXSHA { void _f38f614fa995; };
struct intel_device_info_runtime_init__ATTRS { };
