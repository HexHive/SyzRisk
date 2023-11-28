int __io_scm_file_account(struct io_ring_ctx *ctx, struct file *file)
{

	struct sock *sk = ctx->ring_sock->sk;
	struct sk_buff_head *head = &sk->sk_receive_queue;
	struct scm_fp_list *fpl;
	struct sk_buff *skb;

	if (likely(!io_file_need_scm(file)))
		return 0;

	/*
	 * See if we can merge this file into an existing skb SCM_RIGHTS
	 * file set. If there's no room, fall back to allocating a new skb
	 * and filling it in.
	 */
	spin_lock_irq(&head->lock);
	skb = skb_peek(head);
	if (skb && UNIXCB(skb).fp->count < SCM_MAX_FD)
		__skb_unlink(skb, head);
	else
		skb = NULL;
	spin_unlock_irq(&head->lock);

	if (!skb) {
		fpl = kzalloc(sizeof(*fpl), GFP_KERNEL);
		if (!fpl)
			return -ENOMEM;

		skb = alloc_skb(0, GFP_KERNEL);
		if (!skb) {
			kfree(fpl);
			return -ENOMEM;
		}

		fpl->user = get_uid(current_user());
		fpl->max = SCM_MAX_FD;
		fpl->count = 0;

		UNIXCB(skb).fp = fpl;
		skb->sk = sk;
		skb->scm_io_uring = 1;
		skb->destructor = unix_destruct_scm;
		refcount_add(skb->truesize, &sk->sk_wmem_alloc);
	}

	fpl = UNIXCB(skb).fp;
	fpl->fp[fpl->count++] = get_file(file);
	unix_inflight(fpl->user, file);
	skb_queue_head(head, skb);
	fput(file);

	return 0;
}

struct __io_scm_file_account__HEXSHA { void _0091bfc81741; };
struct __io_scm_file_account__ATTRS { };
