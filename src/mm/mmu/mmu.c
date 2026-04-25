#include "mmu.h"
#include "../pages/page.h"
#include "uart/uart.h"



void map(PageTable *root, uint64_t va, uint64_t pa, uint64_t bits, int level) {
    if((bits & (PTE_R | PTE_W | PTE_X)) == 0){
        panic("Invalid PTE: No permissions set", __FILE__, __LINE__);
    }
    uint64_t vpn[3] = {
        (va >> 12) & 0x1FF, // VPN[0]
        (va >> 21) & 0x1FF, // VPN[1]
        (va >> 30) & 0x1FF  // VPN[2]
    };

    uint64_t ppn[3] = {
        (pa >> 12) & 0x1FF, // PPN[0]
        (pa >> 21) & 0x1FF, // PPN[1]
        (pa >> 30) & 0x1FF  // PPN[2]
    };
    PageTable *current_table = root;
    for(int i=2;i>level;i--){
    
        uint64_t *pte = &current_table->entries[vpn[i]];
        if(pte_is_invalid(*pte)){
            PageTable *new_table = (PageTable *)page_alloc(1);
            for(int k=0;k<512;k++){
                new_table->entries[k] = 0;
            }
           *pte = ((uint64_t)new_table >> 2) | PTE_V;
        }
        current_table = (PageTable *)((*pte & ~0x3FF) << 2);
        uint64_t entry =  (ppn[2] << 28) |   // PPN[2] sits at bits 53:28
                     (ppn[1] << 19) |   // PPN[1] sits at bits 27:19
                     (ppn[0] << 10) |   // PPN[0] sits at bits 18:10
                     bits           |   // RWX Permissions
                     PTE_V;             // Valid bit
        current_table->entries[vpn[i]] = entry;  
    }
    current_table->entries[vpn[level]] = ((pa >> 12) << 10) | bits | PTE_V;
}

void unmap(PageTable *root){
    for(int lv2 = 0;lv2<512;lv2++){
        uint64_t entry_lv2 = root->entries[lv2];
        if(pte_is_valid(entry_lv2) && pte_is_branch(entry_lv2)){
            PageTable *table_lv1 = (PageTable *)((entry_lv2 & ~0x3FF) << 2);
            for(int lv1 = 0;lv1<512;lv1++){
                uint64_t entry_lv1 = table_lv1->entries[lv1];
                if(pte_is_valid(entry_lv1) && pte_is_branch(entry_lv1)){
                    PageTable *table_lv0 = (PageTable *)((entry_lv1 & ~0x3FF) << 2);
                    page_free(table_lv0);
                }
            }
            page_free(table_lv1);
        }
    }
}

uint64_t virt_to_phys(PageTable *root, uint64_t va){
    uint64_t vpn[3] = {
        (va >> 12) & 0x1FF,
        (va >> 21) & 0x1FF,
        (va >> 30) & 0x1FF
    };
    PageTable *current_table = root;
    for(int i=2;i>=0;i--){
        uint64_t pte = current_table->entries[vpn[i]];
        if (pte_is_invalid(pte)) {
            return 0; // Page Fault! Path is a dead end.
        }
        if(pte_is_leaf(pte)){
            int shift_amount = 12 + i * 9; // Calculate the shift based on the level
            uint64_t off_mask = (1ULL << shift_amount) - 1; // Mask for the offset within the page
            uint64_t vaddr_pgoff = va & off_mask;
            uint64_t base_addr = (pte << 2) & ~off_mask;
            return base_addr | vaddr_pgoff;

        }
        current_table = (PageTable *)((pte & ~0x3FF) << 2);
    }
    return 0; // Should never reach here if the page tables are correctly set up.
}

void id_map_range(PageTable *root, uint64_t start, uint64_t end, uint64_t bits){
    uint64_t memaddr = start & ~(PAGE_SIZE - 1);
    uint64_t end_aligned = (end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    while(memaddr < end_aligned){
        map(root, memaddr, memaddr, bits | PTE_A | PTE_D, 0);
        memaddr += PAGE_SIZE;
    }
}

// This function "walks" the table and prints the raw PTE hex value
void debug_pte(PageTable *root, uint64_t va) {
    uint64_t vpn[3] = { (va >> 12) & 0x1FF, (va >> 21) & 0x1FF, (va >> 30) & 0x1FF };
    PageTable *current = root;

    for (int i = 2; i >= 0; i--) {
        uint64_t pte = current->entries[vpn[i]];
        uart_printf("Level %d, Index %d: PTE = 0x%x\r\n", i, vpn[i], pte);

        if (!(pte & PTE_V)) {
            uart_puts("  [!] PTE Invalid! Path ends here.\r\n");
            return;
        }
        if (pte & (PTE_R | PTE_W | PTE_X)) {
            uart_puts("  [!] Found Leaf PTE.\r\n");
            return;
        }
        current = (PageTable *)((pte & ~0x3FF) << 2);
    }
}