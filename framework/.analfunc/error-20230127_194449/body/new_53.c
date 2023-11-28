int uvcg_video_enable(struct uvc_video *video, int enable)
{
	unsigned int i;
	int ret;

	if (video->ep == NULL) {
		uvcg_info(&video->uvc->func,
			  "Video enable failed, device is uninitialized.\n");
		return -ENODEV;
	}

	if (!enable) {
		cancel_work_sync(&video->pump);
		uvcg_queue_cancel(&video->queue, 0);

		for (i = 0; i < video->uvc_num_requests; ++i)
			if (video->ureq && video->ureq[i].req)
				usb_ep_dequeue(video->ep, video->ureq[i].req);

		uvc_video_free_requests(video);
		uvcg_queue_enable(&video->queue, 0);
		return 0;
	}

	if ((ret = uvcg_queue_enable(&video->queue, 1)) < 0)
		return ret;

	if ((ret = uvc_video_alloc_requests(video)) < 0)
		return ret;

	if (video->max_payload_size) {
		video->encode = uvc_video_encode_bulk;
		video->payload_size = 0;
	} else
		video->encode = video->queue.use_sg ?
			uvc_video_encode_isoc_sg : uvc_video_encode_isoc;

	video->req_int_count = 0;

	queue_work(video->async_wq, &video->pump);

	return ret;
}

struct uvcg_video_enable__HEXSHA { void _88c8e05ed5c0; };
struct uvcg_video_enable__ATTRS { };
