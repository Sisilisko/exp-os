#include "common/types.h"

#include "common/memory.h"


static physaddr_t pfa_region_start;
static size_t pfa_page_count;
static size_t pfa_bitmap_words;
static BITMAP_WORD *pfa_bitmap;

static inline void set_bit(size_t bit) { pfa_bitmap[bit / 64] |= (1ULL << (bit % 64)); }
static inline void clr_bit(size_t bit) { pfa_bitmap[bit / 64] &= ~(1ULL << (bit % 64)); }
static inline int test_bit(size_t bit) { return (pfa_bitmap[bit / 64] >> (bit % 64)) & 1ULL;}

void pfa_init(physaddr_t region_start, size_t page_count, void *bitmap_virt_addr) {
    pfa_region_start = region_start;
    pfa_page_count = page_count;
    pfa_bitmap = (BITMAP_WORD *)bitmap_virt_addr;
    pfa_bitmap_words = (page_count + 63) / 64;
    memset(pfa_bitmap, 0, pfa_bitmap_words * sizeof(BITMAP_WORD)); // Everything is free! 
}

physaddr_t alloc_page(void) {
    for (size_t word = 0; word < pfa_bitmap_words; word++)
    {
        if (pfa_bitmap[word] != ~0ULL) // Does this machine have at least one free bit?
        {
            uint64_t w = pfa_bitmap[word]; 
            for (int b = 0; b < 64; b++)
            {
                if (!(w & (1ULL << b)))
                {
                    size_t bit = word * 64 + b;
                    if (bit >= pfa_page_count) return 0;
                    set_bit(bit);
                    return pfa_region_start + (physaddr_t)bit *PAGE_SIZE;
                }
                
            }
            
        }
        
    }
    return 0; // This is returned when we are out of available pages!
}

// Free page (physical)
void free_page(physaddr_t paddr) {
    if (paddr < pfa_region_start) return;
    size_t bit = (paddr - pfa_region_start) / PAGE_SIZE;
    if (bit >= pfa_page_count) return;
    clr_bit(bit); 
}