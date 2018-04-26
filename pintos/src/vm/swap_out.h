/*
#include <list.h>
#include "devices/block.h"

//supplment page table
struct supplement_page{
	uint8_t *u_address;
	struct block *disk_no;
//	uint8_t disk_no;
//	uint8_t *disk_address;
	block_sector_t disk_address;
	uint32_t size;
	struct list_elem elem;
};


//frame table
struct frame{
	uint8_t *address; //user page no
	char referred; //if referred 0, else 0
	struct list_elem elem;

};

//struct frame *frame_table;
//struct frame *victim_pointer;
static struct list frame_table;
static struct list_elem *victim_pointer;

struct swap_table{
	struct lock *lock; //필요할까?
	struct bitmap *used_space;
//	uint8_t *base;

};

static struct block *swap_disk;

struct swap_table swap_tb;

void frame_init();
void frame_insert(uint8_t *address);

uint8_t *LRU_page_out(bool dirty);
void SPT_insert(uint8_t *u_address, struct block * disk_no, block_sector_t disk_address);
void SPT_search_disk(uint8_t *uaddress, uint8_t *disk_no, uint32_t *disk_address);
void swap_tb_init();
void frame_remove();
uint8_t* LRU_page_out(bool dirty);
void swap_out (uint8_t *kpage, bool dirty);


*/
