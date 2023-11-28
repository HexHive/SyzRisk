static void __releases(tfile->lock) 

{
	struct ttm_ref_object *ref =
	    container_of(kref, struct ttm_ref_object, kref);
	struct ttm_object_file *tfile = ref->tfile;
	struct vmwgfx_open_hash *ht;

	ht = &tfile->ref_hash;
	(void)vmwgfx_ht_remove_item_rcu(ht, &ref->hash);
	list_del(&ref->head);
	spin_unlock(&tfile->lock);

	ttm_base_object_unref(&ref->obj);
	kfree_rcu(ref, rcu_head);
	spin_lock(&tfile->lock);
}

struct __releases__HEXSHA { void _76a9e07f270c; };
struct __releases__ATTRS { void ___acquires; void ___releases; };
