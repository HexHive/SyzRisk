static bool
cfg80211_update_known_bss(struct cfg80211_registered_device *rdev,
			  struct cfg80211_internal_bss *known,
			  struct cfg80211_internal_bss *new,
			  bool signal_valid)
{
	lockdep_assert_held(&rdev->bss_lock);

	/* Update IEs */
	if (rcu_access_pointer(new->pub.proberesp_ies)) {
		const struct cfg80211_bss_ies *old;

		old = rcu_access_pointer(known->pub.proberesp_ies);

		rcu_assign_pointer(known->pub.proberesp_ies,
				   new->pub.proberesp_ies);
		/* Override possible earlier Beacon frame IEs */
		rcu_assign_pointer(known->pub.ies,
				   new->pub.proberesp_ies);
		if (old)
			kfree_rcu((struct cfg80211_bss_ies *)old, rcu_head);
	} else if (rcu_access_pointer(new->pub.beacon_ies)) {
		const struct cfg80211_bss_ies *old;

		if (known->pub.hidden_beacon_bss &&
		    !list_empty(&known->hidden_list)) {
			const struct cfg80211_bss_ies *f;

			/* The known BSS struct is one of the probe
			 * response members of a group, but we're
			 * receiving a beacon (beacon_ies in the new
			 * bss is used). This can only mean that the
			 * AP changed its beacon from not having an
			 * SSID to showing it, which is confusing so
			 * drop this information.
			 */

			f = rcu_access_pointer(new->pub.beacon_ies);
			kfree_rcu((struct cfg80211_bss_ies *)f, rcu_head);
			return false;
		}

		old = rcu_access_pointer(known->pub.beacon_ies);

		rcu_assign_pointer(known->pub.beacon_ies, new->pub.beacon_ies);

		/* Override IEs if they were from a beacon before */
		if (old == rcu_access_pointer(known->pub.ies))
			rcu_assign_pointer(known->pub.ies, new->pub.beacon_ies);

		cfg80211_update_hidden_bsses(known,
					     rcu_access_pointer(new->pub.beacon_ies),
					     old);

		if (old)
			kfree_rcu((struct cfg80211_bss_ies *)old, rcu_head);
	}

	known->pub.beacon_interval = new->pub.beacon_interval;

	/* don't update the signal if beacon was heard on
	 * adjacent channel.
	 */
	if (signal_valid)
		known->pub.signal = new->pub.signal;
	known->pub.capability = new->pub.capability;
	known->ts = new->ts;
	known->ts_boottime = new->ts_boottime;
	known->parent_tsf = new->parent_tsf;
	known->pub.chains = new->pub.chains;
	memcpy(known->pub.chain_signal, new->pub.chain_signal,
	       IEEE80211_MAX_CHAINS);
	ether_addr_copy(known->parent_bssid, new->parent_bssid);
	known->pub.max_bssid_indicator = new->pub.max_bssid_indicator;
	known->pub.bssid_index = new->pub.bssid_index;

	return true;
}

struct cfg80211_update_known_bss__HEXSHA { void _03c0ad4b06c3; };
struct cfg80211_update_known_bss__ATTRS { };
