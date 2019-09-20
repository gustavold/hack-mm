#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gustavo Luiz Duarte");
MODULE_DESCRIPTION("Just hack it!");
MODULE_VERSION("0.01");

#define PID 10887
#define old_addr 0x7fffb4f50000

#define new_addr 0x140000000

void (*my_set_pte_at)(struct mm_struct *mm, unsigned long addr, pte_t *ptep, pte_t pte) = 0xc00000000007bea0;
pte_t *(*__my_get_locked_pte)(struct mm_struct *mm, unsigned long addr, spinlock_t **ptl) = 0xc000000000417100;


static inline pte_t *my_get_locked_pte(struct mm_struct *mm, unsigned long addr,
                                    spinlock_t **ptl)
{
        pte_t *ptep;
        __cond_lock(*ptl, ptep = __my_get_locked_pte(mm, addr, ptl));
        return ptep;
}

static struct page *walk_page_table(struct mm_struct *mm, unsigned long addr)
{
	pgd_t *pgd;
	pte_t *ptep, pte;
	pud_t *pud;
	pmd_t *pmd;

	struct page *page = NULL;

	pgd = pgd_offset(mm, addr);
	if (pgd_none(*pgd) || pgd_bad(*pgd))
		goto out;

	pud = pud_offset(pgd, addr);
	if (pud_none(*pud) || pud_bad(*pud))
		goto out;

	pmd = pmd_offset(pud, addr);
	if (pmd_none(*pmd) || pmd_bad(*pmd))
		goto out;

	ptep = pte_offset_map(pmd, addr);
	if (!ptep)
		goto out;
	pte = *ptep;

	page = pte_page(pte);
	if (page)
		printk(KERN_INFO "page frame struct is @ %p", page);

	pte_unmap(ptep);
out:
	return page;
}

struct task_struct *task;

static int __init hack_mm_init(void) {
	struct page *page;
	spinlock_t *ptl;
	pte_t *pte;

	printk(KERN_INFO "Hello, World!\n");
	task = pid_task(find_get_pid(PID), PIDTYPE_PID);
	page = walk_page_table(task->mm, old_addr);
	printk(KERN_INFO "pfn of old_addr: %lu\n", page_to_pfn(page));
	pte = my_get_locked_pte(task->mm, new_addr, &ptl);
	my_set_pte_at(task->mm, new_addr, pte, mk_pte(page, task->mm->mmap->vm_page_prot));
	spin_unlock(ptl);

	page = walk_page_table(task->mm, new_addr);
	printk(KERN_INFO "pfn of new_addr: %lu\n", page_to_pfn(page));

	return 0;
}
static void __exit hack_mm_exit(void) {
	printk(KERN_INFO "Goodbye, World!\n");
}
module_init(hack_mm_init);
module_exit(hack_mm_exit);
