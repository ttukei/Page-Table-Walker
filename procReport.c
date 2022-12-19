#include<linux/module.h>
#include<linux/sched/signal.h>
#include<linux/pid_namespace.h> 
#include<asm/io.h> 

unsigned long virtu2phys(struct mm_struct *mm, unsigned long vpage) {
  unsigned long physical_page_addr;
  pgd_t *pgd;
  p4d_t *p4d;
  pud_t *pud;
  pmd_t *pmd;
  pte_t *pte;
  struct page *page;
  pgd = pgd_offset(mm, vpage);
  if (pgd_none(*pgd) || pgd_bad(*pgd))
    return 0;
  p4d = p4d_offset(pgd, vpage);
  if (p4d_none(*p4d) || p4d_bad(*p4d))
    return 0;
  pud = pud_offset(p4d, vpage);
  if (pud_none(*pud) || pud_bad(*pud))
    return 0;
  pmd = pmd_offset(pud, vpage);
  if (pmd_none(*pmd) || pmd_bad(*pmd))
    return 0;
  if (! (pte = pte_offset_map(pmd, vpage)))
    return 0;
  if (! (page = pte_page(*pte)))
    return 0;
  physical_page_addr = page_to_phys(page);
  pte_unmap(pte);
  // handle unmapped pages
  if (physical_page_addr==70368744173568)
    return 0;
  return physical_page_addr;
}

int proc_count(void)
{
  int i=0, counter = 0, contigCounter = 0, nonContigCounter = 0;
  struct task_struct *task;
  struct vm_area_struct *vma = 0;
  unsigned long vpage;
  printk("PROCESS REPORT:\n");
  printk("proc_id,proc_name,contig_pages,noncontig_pages,total_pages\n");
  for_each_process(task)
  {
    int totalPages = 0;
    int contig = 0, nonContig = 0;
    if (task->pid > 650) {
      if (task->mm && task->mm->mmap) {
        unsigned long previous_page_addr;
        for (vma = task->mm->mmap; vma; vma = vma->vm_next) {
          for (vpage = vma->vm_start; vpage < vma->vm_end; vpage += PAGE_SIZE){
            unsigned long physical_page_addr = virtu2phys(task->mm, vpage);
            if (physical_page_addr != 0) {
              if (physical_page_addr == previous_page_addr + PAGE_SIZE) {
                contig++;
              } else {
                nonContig++;
              }
              previous_page_addr = physical_page_addr;
              totalPages++;
            }
          }
        }
      }
      counter += totalPages;
      printk("%d,%s,%d,%d,%d\n",task->pid, task->comm, contig, nonContig, totalPages);
      i++;
    }
    contigCounter += contig;
    nonContigCounter += nonContig;
  }
  printk("TOTALS,%d,%d,%d\n", contigCounter, nonContigCounter, counter);
  return i;
}

int proc_init (void) {
  printk(KERN_INFO "helloModule: kernel module initialized\n");
  printk(KERN_INFO "There are %d running processes.\n", proc_count()); 
  return 0;
}

void proc_cleanup(void) {
  printk(KERN_INFO "helloModule: performing cleanup of module\n");
}


MODULE_LICENSE("GPL");
module_init(proc_init);
module_exit(proc_cleanup);

