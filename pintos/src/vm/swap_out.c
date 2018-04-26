/*#include "swap_out.h"

void frame_init()
{
	list_init(&frame_table);
}
void frame_insert(uint8_t *address)
{
	struct frame *f;
	f = (struct frame *)malloc(sizeof(struct frame));
	f->address = address;
	f->referred = 1;
	list_insert(victim_pointer, &f->elem);
	victim_pointer = list_next(victim_pointer);

}

uint8_t *LRU_page_out(bool dirty)
{
	struct list_elem *e;
	struct frame *f;
	uint8_t *address;
	//LRU second chance algoirthm
	for(e=victim_pointer;
			;
			e = list_next_circular(e, &frame_table)	)
	{
		f = list_entry(e, struct frame, elem);
		if(f->referred == 0)
		{
			//least unused bit, swap out this frame
			swap_out(f->address, dirty);
			//address = f->address;
			palloc_free_page(f->address); //page empty
			victim_pointer = list_prev(list_remove(e));
			free(f);
			//return address;
		}
		else if(f->referred == 1)
		{
			f->referred = 0;
		}

		//circular queue
	///	if(e==list_prev (list_end(&frame_table)))
//			e = list_prev (list_begin(&frame_table));

	}

}


//supplement page table

//swap out / load / setup stack때 할당할거임.
void SPT_insert(uint8_t *u_address, struct block * disk_no, block_sector_t disk_address)
{
	struct supplement_page *sp;
	sp = (struct supplement_page *)malloc(sizeof(supplement_page));
	sp->u_address = u_address;
	sp->disk_no = disk_no;
	sp->disk_address = disk_address;
	
	list_push_back(&sp->elem);
}

void SPT_search_disk(uint8_t *uaddress, uint8_t *disk_no, uint32_t *disk_address)
{
	struct list_elem *e;
	struct supplement_page *sp;

	for(e=list_begin(thread_current()->spt_elem);
			e!=list_end(thread_current()->spt_elem);
			e = list_next(e)	)
	{
		sp = list_entry(e, struct supplement_page, elem);			
		if(sp->u_address == uaddress)
		{
			*disk_no = sp->disk_no;
			*disk_address = sp->disk_address;
		}			
	}
}
//free는 나중에 구현! 굳이 안구현해도 될수도 있을듯

//swap table
void swap_tb_init()
{
	swap_disk = block_get_role(BLOCK_SWAP);
	swap_tb->used_space = bitmap_create(swap_disk->size);
}

void swap_out (uint8_t *kpage, bool dirty)
{
	block_sector_t block_addr;
	//need bitmap
	if(dirty)
	{
		//check the swap table은 안해도됨	
		//0인 위치 가져오고 1로바꾼다.
		//bring the first 0(empty block) and change it to 1
		block_addr = bitmap_scan_and_flip (swap->table, 0, 1, false);			
	//페이지 write가 필요한데, 얘는 얼마나 write하지?
		block_write(swap_disk, block_addr, kpage);


	}	
}
*/
