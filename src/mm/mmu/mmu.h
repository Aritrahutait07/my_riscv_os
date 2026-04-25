#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include <stdbool.h>

#define csrw(reg, val) ({ \
    __asm__ volatile ("csrw " #reg ", %0" :: "rK"(val)); })

#define csrr(reg) ({ \
    uint64_t __val; \
    __asm__ volatile ("csrr %0, " #reg : "=r"(__val)); \
    __val; })
#define PTE_V (1 << 0) // Valid
#define PTE_R (1 << 1) // Read
#define PTE_W (1 << 2) // Write
#define PTE_X (1 << 3) // Execute
#define PTE_U (1 << 4) // User
#define PTE_A (1 << 6) // Accessed
#define PTE_D (1 << 7) // Dirty


typedef struct{
    uint64_t entries[512]; // Each page table has 512 entries, and each entry is 64 bits (8 bytes) in size, which allows for a total of 4KB (4096 bytes) per page table. This structure represents a single level of the page table hierarchy in a virtual memory system.
} PageTable;

static inline bool pte_is_valid(uint64_t pte) {
    return (pte & PTE_V) != 0; 
}

static inline bool pte_is_invalid(uint64_t pte) {
    return !pte_is_valid(pte);
}

static inline bool pte_is_leaf(uint64_t pte) {
    return (pte & (PTE_R | PTE_W | PTE_X)) != 0;
}

static inline bool pte_is_branch(uint64_t pte) {
    return pte_is_valid(pte) && !pte_is_leaf(pte);
}


void map(PageTable *root, uint64_t va, uint64_t pa, uint64_t bits, int level);
uint64_t virt_to_phys(PageTable *root, uint64_t va);
void id_map_range(PageTable *root, uint64_t start, uint64_t end, uint64_t bits);
void unmap(PageTable *root);
void debug_pte(PageTable *root, uint64_t va);
#endif
