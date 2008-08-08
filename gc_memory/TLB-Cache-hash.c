/* TLB-Cache-hash.c - This is how the TLB LUT should be accessed, using a hash map
   by Mike Slegeir for Mupen64-GC
   ----------------------------------------------------
   FIXME: This should be profiled to determine the best size,
            currently, the linked lists' length can be up to ~16,000
   ----------------------------------------------------
   MEMORY USAGE:
     STATIC:
     	TLB LUT r: NUM_SLOTS * 4 (currently 256 bytes)
     	TLB LUT w: NUM_SLOTS * 4 (currently 256 bytes)
     HEAP:
     	TLB hash nodes: 2 (r/w) * 12 bytes * O( 2^20 ) entries
 */

#include <stdlib.h>
#include <assert.h>
#include "TLB-Cache.h"

#ifdef USE_TLB_CACHE

static unsigned int TLB_hash_shift;

void TLBCache_init(void){
	unsigned int temp = TLB_NUM_SLOTS;
	while(temp){
		temp >>= 1;
		++TLB_hash_shift;
	}
	TLB_hash_shift = TLB_BITS_PER_PAGE_NUM - TLB_hash_shift + 1;
}

void TLBCache_deinit(void){
	int i;
	TLB_hash_node* node, * next;
	for(i=0; i<TLB_NUM_SLOTS; ++i){
		// Clear r
		for(node = TLB_LUT_r[i]; node != NULL; node = next){
			next = node->next;
			free(node);
		}
		TLB_LUT_r[i] = NULL;
		
		// Clear w
		for(node = TLB_LUT_w[i]; node != NULL; node = next){
			next = node->next;
			free(node);
		}
		TLB_LUT_w[i] = NULL;
	}
}

static unsigned int inline TLB_hash(unsigned int page){
	return page >> TLB_hash_shift;
}

unsigned int inline TLBCache_get_r(unsigned int page){
	TLB_hash_node* node = TLB_LUT_r[ TLB_hash(page) ];
	
	for(; node != NULL; node = node->next)
		if(node->page == page) return node->value;
	
	return 0;
}

unsigned int inline TLBCache_get_w(unsigned int page){
	TLB_hash_node* node = TLB_LUT_w[ TLB_hash(page) ];
	
	for(; node != NULL; node = node->next)
		if(node->page == page) return node->value;
	
	return 0;
}

void inline TLBCache_set_r(unsigned int page, unsigned int val){
	TLB_hash_node* next = TLB_LUT_r[ TLB_hash(page) ];
	
	TLB_hash_node* node = malloc( sizeof(TLB_hash_node) );
	node->page  = page;
	node->value = val;
	node->next  = next;
	TLB_LUT_r[ TLB_hash(page) ] = node;
	
	// Remove any old values for this page from the linked list
	for(; node != NULL; node = node->next)
		if(node->next != NULL && node->next->page == page){
			TLB_hash_node* old_node = node->next;
			node->next = old_node->next;
			free( old_node );
			break;
		}
}

void inline TLBCache_set_w(unsigned int page, unsigned int val){
	TLB_hash_node* next = TLB_LUT_w[ TLB_hash(page) ];
	
	TLB_hash_node* node = malloc( sizeof(TLB_hash_node) );
	node->page  = page;
	node->value = val;
	node->next  = next;
	TLB_LUT_w[ TLB_hash(page) ] = node;
	
	// Remove any old values for this page from the linked list
	for(; node != NULL; node = node->next)
		if(node->next != NULL && node->next->page == page){
			TLB_hash_node* old_node = node->next;
			node->next = old_node->next;
			free( old_node );
			break;
		}
}


#endif
