static int hisi_sas_abort_task(struct sas_task *task)
{
	struct hisi_sas_internal_abort_data internal_abort_data = { false };
	struct domain_device *device = task->dev;
	struct hisi_sas_device *sas_dev = device->lldd_dev;
	struct hisi_hba *hisi_hba;
	struct device *dev;
	int rc = TMF_RESP_FUNC_FAILED;
	unsigned long flags;

	if (!sas_dev)
		return TMF_RESP_FUNC_FAILED;

	hisi_hba = dev_to_hisi_hba(task->dev);
	dev = hisi_hba->dev;

	spin_lock_irqsave(&task->task_state_lock, flags);
	if (task->task_state_flags & SAS_TASK_STATE_DONE) {
		struct hisi_sas_slot *slot = task->lldd_task;
		struct hisi_sas_cq *cq;

		if (slot) {
			/*
			 * sync irq to avoid free'ing task
			 * before using task in IO completion
			 */
			cq = &hisi_hba->cq[slot->dlvry_queue];
			synchronize_irq(cq->irq_no);
		}
		spin_unlock_irqrestore(&task->task_state_lock, flags);
		rc = TMF_RESP_FUNC_COMPLETE;
		goto out;
	}
	task->task_state_flags |= SAS_TASK_STATE_ABORTED;
	spin_unlock_irqrestore(&task->task_state_lock, flags);

	if (task->lldd_task && task->task_proto & SAS_PROTOCOL_SSP) {
		struct hisi_sas_slot *slot = task->lldd_task;
		u16 tag = slot->idx;
		int rc2;

		rc = sas_abort_task(task, tag);
		rc2 = sas_execute_internal_abort_single(device, tag,
				slot->dlvry_queue, &internal_abort_data);
		if (rc2 < 0) {
			dev_err(dev, "abort task: internal abort (%d)\n", rc2);
			return TMF_RESP_FUNC_FAILED;
		}

		/*
		 * If the TMF finds that the IO is not in the device and also
		 * the internal abort does not succeed, then it is safe to
		 * free the slot.
		 * Note: if the internal abort succeeds then the slot
		 * will have already been completed
		 */
		if (rc == TMF_RESP_FUNC_COMPLETE && rc2 != TMF_RESP_FUNC_SUCC) {
			if (task->lldd_task)
				hisi_sas_do_release_task(hisi_hba, task, slot);
		}
	} else if (task->task_proto & SAS_PROTOCOL_SATA ||
		task->task_proto & SAS_PROTOCOL_STP) {
		if (task->dev->dev_type == SAS_SATA_DEV) {
			rc = hisi_sas_internal_task_abort_dev(sas_dev, false);
			if (rc < 0) {
				dev_err(dev, "abort task: internal abort failed\n");
				goto out;
			}
			hisi_sas_dereg_device(hisi_hba, device);
			rc = hisi_sas_softreset_ata_disk(device);
		}
	} else if (task->lldd_task && task->task_proto & SAS_PROTOCOL_SMP) {
		/* SMP */
		struct hisi_sas_slot *slot = task->lldd_task;
		u32 tag = slot->idx;
		struct hisi_sas_cq *cq = &hisi_hba->cq[slot->dlvry_queue];

		rc = sas_execute_internal_abort_single(device,
						       tag, slot->dlvry_queue,
						       &internal_abort_data);
		if (((rc < 0) || (rc == TMF_RESP_FUNC_FAILED)) &&
					task->lldd_task) {
			/*
			 * sync irq to avoid free'ing task
			 * before using task in IO completion
			 */
			synchronize_irq(cq->irq_no);
			slot->task = NULL;
		}
	}

out:
	if (rc != TMF_RESP_FUNC_COMPLETE)
		dev_notice(dev, "abort task: rc=%d\n", rc);
	return rc;
}

struct hisi_sas_abort_task__HEXSHA { void _4b329abc9180; };
struct hisi_sas_abort_task__ATTRS { };
