void unix_gc(void)
{
	struct sk_buff *next_skb, *skb;
	struct unix_sock *u;
	struct unix_sock *next;
	struct sk_buff_head hitlist;
	struct list_head cursor;
	LIST_HEAD(not_cycle_list);

	spin_lock(&unix_gc_lock);

	/* Avoid a recursive GC. */
	if (gc_in_progress)
		goto out;

	/* Paired with READ_ONCE() in wait_for_unix_gc(). */
	WRITE_ONCE(gc_in_progress, true);

	/* First, select candidates for garbage collection.  Only
	 * in-flight sockets are considered, and from those only ones
	 * which don't have any external reference.
	 *
	 * Holding unix_gc_lock will protect these candidates from
	 * being detached, and hence from gaining an external
	 * reference.  Since there are no possible receivers, all
	 * buffers currently on the candidates' queues stay there
	 * during the garbage collection.
	 *
	 * We also know that no new candidate can be added onto the
	 * receive queues.  Other, non candidate sockets _can_ be
	 * added to queue, so we must make sure only to touch
	 * candidates.
	 */
	list_for_each_entry_safe(u, next, &gc_inflight_list, link) {
		long total_refs;
		long inflight_refs;

		total_refs = file_count(u->sk.sk_socket->file);
		inflight_refs = atomic_long_read(&u->inflight);

		BUG_ON(inflight_refs < 1);
		BUG_ON(total_refs < inflight_refs);
		if (total_refs == inflight_refs) {
			list_move_tail(&u->link, &gc_candidates);
			__set_bit(UNIX_GC_CANDIDATE, &u->gc_flags);
			__set_bit(UNIX_GC_MAYBE_CYCLE, &u->gc_flags);
		}
	}

	/* Now remove all internal in-flight reference to children of
	 * the candidates.
	 */
	list_for_each_entry(u, &gc_candidates, link)
		scan_children(&u->sk, dec_inflight, NULL);

	/* Restore the references for children of all candidates,
	 * which have remaining references.  Do this recursively, so
	 * only those remain, which form cyclic references.
	 *
	 * Use a "cursor" link, to make the list traversal safe, even
	 * though elements might be moved about.
	 */
	list_add(&cursor, &gc_candidates);
	while (cursor.next != &gc_candidates) {
		u = list_entry(cursor.next, struct unix_sock, link);

		/* Move cursor to after the current position. */
		list_move(&cursor, &u->link);

		if (atomic_long_read(&u->inflight) > 0) {
			list_move_tail(&u->link, &not_cycle_list);
			__clear_bit(UNIX_GC_MAYBE_CYCLE, &u->gc_flags);
			scan_children(&u->sk, inc_inflight_move_tail, NULL);
		}
	}
	list_del(&cursor);

	/* Now gc_candidates contains only garbage.  Restore original
	 * inflight counters for these as well, and remove the skbuffs
	 * which are creating the cycle(s).
	 */
	skb_queue_head_init(&hitlist);
	list_for_each_entry(u, &gc_candidates, link)
		scan_children(&u->sk, inc_inflight, &hitlist);

	/* not_cycle_list contains those sockets which do not make up a
	 * cycle.  Restore these to the inflight list.
	 */
	while (!list_empty(&not_cycle_list)) {
		u = list_entry(not_cycle_list.next, struct unix_sock, link);
		__clear_bit(UNIX_GC_CANDIDATE, &u->gc_flags);
		list_move_tail(&u->link, &gc_inflight_list);
	}

	spin_unlock(&unix_gc_lock);

	/* We need io_uring to clean its registered files, ignore all io_uring
	 * originated skbs. It's fine as io_uring doesn't keep references to
	 * other io_uring instances and so killing all other files in the cycle
	 * will put all io_uring references forcing it to go through normal
	 * release.path eventually putting registered files.
	 */
	skb_queue_walk_safe(&hitlist, skb, next_skb) {
		if (skb->scm_io_uring) {
			__skb_unlink(skb, &hitlist);
			skb_queue_tail(&skb->sk->sk_receive_queue, skb);
		}
	}

	/* Here we are. Hitlist is filled. Die. */
	__skb_queue_purge(&hitlist);

	spin_lock(&unix_gc_lock);

	/* There could be io_uring registered files, just push them back to
	 * the inflight list
	 */
	list_for_each_entry_safe(u, next, &gc_candidates, link)
		list_move_tail(&u->link, &gc_inflight_list);

	/* All candidates should have been detached by now. */
	BUG_ON(!list_empty(&gc_candidates));

	/* Paired with READ_ONCE() in wait_for_unix_gc(). */
	WRITE_ONCE(gc_in_progress, false);

	wake_up(&unix_gc_wait);

 out:
	spin_unlock(&unix_gc_lock);
}

struct unix_gc__HEXSHA { void _0091bfc81741; };
struct unix_gc__ATTRS { };
