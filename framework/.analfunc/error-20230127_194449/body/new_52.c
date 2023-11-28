static int uvc_buffer_prepare(struct vb2_buffer *vb)
{
	struct uvc_video_queue *queue = vb2_get_drv_priv(vb->vb2_queue);
	struct vb2_v4l2_buffer *vbuf = to_vb2_v4l2_buffer(vb);
	struct uvc_buffer *buf = container_of(vbuf, struct uvc_buffer, buf);

	if (vb->type == V4L2_BUF_TYPE_VIDEO_OUTPUT &&
	    vb2_get_plane_payload(vb, 0) > vb2_plane_size(vb, 0)) {
		uvc_trace(UVC_TRACE_CAPTURE, "[E] Bytes used out of bounds.\n");
		return -EINVAL;
	}

	if (unlikely(queue->flags & UVC_QUEUE_DISCONNECTED))
		return -ENODEV;

	buf->state = UVC_BUF_STATE_QUEUED;
	if (queue->use_sg) {
		buf->sgt = vb2_dma_sg_plane_desc(vb, 0);
		buf->sg = buf->sgt->sgl;
	} else {
		buf->mem = vb2_plane_vaddr(vb, 0);
	}
	buf->length = vb2_plane_size(vb, 0);
	if (vb->type == V4L2_BUF_TYPE_VIDEO_CAPTURE)
		buf->bytesused = 0;
	else
		buf->bytesused = vb2_get_plane_payload(vb, 0);

	return 0;
}

struct uvc_buffer_prepare__HEXSHA { void _88c8e05ed5c0; };
struct uvc_buffer_prepare__ATTRS { };
