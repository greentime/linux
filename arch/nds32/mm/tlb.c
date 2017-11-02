/*
 * Copyright (C) 2005-2017 Andes Technology Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/spinlock_types.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <asm/nds32.h>
#include <nds32_intrinsic.h>

unsigned int cpu_last_cid = { TLB_MISC_mskCID + (2 << TLB_MISC_offCID) };

DEFINE_SPINLOCK(cid_lock);

void local_flush_tlb_range(struct vm_area_struct *vma,
			   unsigned long start, unsigned long end)
{
	unsigned long flags, ocid, ncid;

	if ((end - start) > 0x400000) {
		__nds32__tlbop_flua();
		__nds32__isb();
		return;
	}

	spin_lock_irqsave(&cid_lock, flags);
	ocid = __nds32__mfsr(NDS32_SR_TLB_MISC);
	ncid = (ocid & ~TLB_MISC_mskCID) | vma->vm_mm->context.id;
	__nds32__mtsr_dsb(ncid, NDS32_SR_TLB_MISC);
	while (start < end) {
		__nds32__tlbop_inv(start);
		__nds32__isb();
		start += PAGE_SIZE;
	}
	__nds32__mtsr_dsb(ocid, NDS32_SR_TLB_MISC);
	spin_unlock_irqrestore(&cid_lock, flags);
}

void local_flush_tlb_page(struct vm_area_struct *vma, unsigned long addr)
{
	unsigned long flags, ocid, ncid;

	spin_lock_irqsave(&cid_lock, flags);
	ocid = __nds32__mfsr(NDS32_SR_TLB_MISC);
	ncid = (ocid & ~TLB_MISC_mskCID) | vma->vm_mm->context.id;
	__nds32__mtsr_dsb(ncid, NDS32_SR_TLB_MISC);
	__nds32__tlbop_inv(addr);
	__nds32__isb();
	__nds32__mtsr_dsb(ocid, NDS32_SR_TLB_MISC);
	spin_unlock_irqrestore(&cid_lock, flags);
}
