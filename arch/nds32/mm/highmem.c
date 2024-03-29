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

#include <linux/export.h>
#include <linux/highmem.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/interrupt.h>
#include <linux/bootmem.h>
#include <asm/fixmap.h>
#include <asm/tlbflush.h>

void *kmap(struct page *page)
{
	unsigned long vaddr;
	might_sleep();
	if (!PageHighMem(page))
		return page_address(page);
	vaddr = (unsigned long)kmap_high(page);
	return (void *)vaddr;
}

EXPORT_SYMBOL(kmap);

void kunmap(struct page *page)
{
	BUG_ON(in_interrupt());
	if (!PageHighMem(page))
		return;
	kunmap_high(page);
}

EXPORT_SYMBOL(kunmap);

void *kmap_atomic(struct page *page)
{
	unsigned int idx;
	unsigned long vaddr, pte;
	int type;
	pte_t *ptep;

	preempt_disable();
	pagefault_disable();
	if (!PageHighMem(page))
		return page_address(page);

	type = kmap_atomic_idx_push();

	idx = type + KM_TYPE_NR * smp_processor_id();
	vaddr = __fix_to_virt(FIX_KMAP_BEGIN + idx);
	pte = (page_to_pfn(page) << PAGE_SHIFT) | (PAGE_KERNEL);
	ptep = pte_offset_kernel(pmd_off_k(vaddr), vaddr);
	set_pte(ptep, pte);

	__nds32__tlbop_inv(vaddr);
	__nds32__mtsr_dsb(vaddr, NDS32_SR_TLB_VPN);
	__nds32__tlbop_rwr(pte);
	__nds32__isb();
	return (void *)vaddr;
}

EXPORT_SYMBOL(kmap_atomic);

void __kunmap_atomic(void *kvaddr)
{
	if (kvaddr >= (void *)FIXADDR_START) {
		unsigned long vaddr = (unsigned long)kvaddr;
		pte_t *ptep;
		kmap_atomic_idx_pop();
		__nds32__tlbop_inv(vaddr);
		__nds32__isb();
		ptep = pte_offset_kernel(pmd_off_k(vaddr), vaddr);
		set_pte(ptep, 0);
	}
	pagefault_enable();
	preempt_enable();
}

EXPORT_SYMBOL(__kunmap_atomic);
