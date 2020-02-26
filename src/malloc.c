/*
Programming Assignment 4 ( MALLOC)
April 26 2019
CONTRIBUTORS
Sudip Ghale
Michael Pena
*/


#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct _block *)(ptr) - 1)


static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct _block
{
   size_t  size;         /* Size of the allocated _block of memory in bytes */
   struct _block *next;  /* Pointer to the next _block of allcated memory   */
   bool   free;          /* Is this _block free?                     */
   char   padding[3];
};


struct _block *freeList = NULL; /* Free list to track the _blocks available */
struct _block *next_block = NULL;
struct _block *next_comp = NULL;

/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 * \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
 */
struct _block *findFreeBlock(struct _block **last, size_t size)
{
   struct _block *curr = freeList;


#if defined FIT && FIT == 0
   /* First fit */
   while (curr && !(curr->free && curr->size >= size))
   {
      *last = curr; // what is purpose of last here???
      curr  = curr->next;
   }
#endif
/*
for best fit, we compare each two adjacent blocks and update the block with smaller size,
repeat untill the end of the blocks and assign best_block to curr block

*/
#if defined BEST && BEST == 0
  printf("TODO: Implement best fit here\n");
   struct _block *best_block = NULL;

   if(curr && (curr->free && curr->size >= size))
   {
     best_block= curr;
     next_comp = curr;
     curr  = curr->next;
   }

   while (curr)
   {
     if( (curr->free && curr->size >= size))
     {
       if((next_comp->size) > (curr->size))
       {
         best_block = curr;
       }
     }
    next_comp = curr;
    curr  = curr->next;
   }
   curr = best_block;


#endif
/*
for worst fit, we compare each two adjacent blocks and update the block with bigger size,
repeat untill the end of the blocks and assign best_block to curr block

*/
#if defined WORST && WORST == 0
 printf("TODO: Implement worst fit here\n"); // NEED TO UPDATE LAST???
   struct _block *worst_block = NULL;


      if(curr && (curr->free && curr->size >= size))
      {
        worst_block= curr;
        next_comp = curr;
        curr  = curr->next;
      }

      while (curr)
      {
        if( (curr->free && curr->size >= size))
        {
          if((next_comp->size) < (curr->size))
          {
            worst_block = curr;
          }

        }
       next_comp = curr;
       curr  = curr->next;
      }
      curr = worst_block;

#endif
/*
for next fit, we keep track of the last block we visited, and loop thorough to find the next first fit blcok until
the end of the blocks and assign best_block to curr block

*/
#if defined NEXT && NEXT == 0
 printf("TODO: Implement next fit here\n");

   curr = next_block; //next_block = NULL
   if(!next_block) //not null
   {
     curr = freeList;
   }
   while (curr && !(curr->free && curr->size >= size))
   {
      *last = curr;
      curr  = curr->next;
      next_block=curr;
   }

#endif
   return curr;
}


/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size)
{
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0);
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1)
   {
      return NULL;
   }

   /* Update freeList if not set */
   if (freeList == NULL)
   {
      freeList = curr;
   }

   /* Attach new _block to prev _block */
   if (last)
   {
      last->next = curr;
   }

   /* Update _block metadata */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
   max_heap +=size;


   return curr;
}

void split_block(struct _block *next, size_t size)
{
  struct _block *split = (struct _block *)sbrk(0);

  split->size = next->size - size - sizeof(struct _block); //subtract size and assign it to new split block
  split->next = next->next; //assign the next block's next block to the split block
  split->free = true; //assign the the block to free so its available to use
  next->next = split; //assign the split block to the list
  next->size = size; //assign the size to next
  num_splits++;
  num_blocks++;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process
 * or NULL if failed
 */
void *malloc(size_t size)
{
  num_requested += size;

   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0)
   {
      return NULL;
   }

   /* Look for free _block */
   struct _block *last = freeList;
   struct _block *next = findFreeBlock(&last, size);

   /* TODO: Split free _block if possible */
  if ( next != NULL)
  {
    if((next->size) > size ) // *****
    {
      split_block(next, size); // call split fun if the size if grean than requested
      next->free = false;
    }
    num_reuses++; // increment the num_reuses
  }

   /* Could not find free _block, so grow heap */
   if (next == NULL)
   {
      next = growHeap(last, size);
      num_grows++; // what is diff in num_grows and num_blocks???
      num_blocks++; // increment the num_blocks
   }

   /* Could not find free _block or grow heap, so just return NULL */
   if (next == NULL)
   {
      return NULL;
   }

   /* Mark _block as in use */
   next->free = false;
   num_mallocs++; // increment the num_mallocs


   /* Return data address associated with _block */
   return BLOCK_DATA(next);
}


//coalesce
void coalesce(struct _block *curr)
{
   curr = freeList;

   while( curr )
   {
     if ( curr && curr->free && curr->next && curr->next->free ) //if two adjecent blocks are free join them together
     {
       curr->size += (curr->next->size) + sizeof(struct _block); //add the size of the current block and the next block
       curr->next = curr->next->next; //from the current blocks next block N get N's next block and add it the current block's next block
       num_coalesces++; // for test3 it should be one
       num_blocks--;// decrement the num_blocks
     }
     curr = curr->next; //next loop
   }
}


/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr)
{
   if (ptr == NULL)
   {
      return;
   }

   /* Make _block as free */
   struct _block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;
   num_frees++;

   //Coalesce
   coalesce(curr);


}
/*
Allocates a block of memory for an array of num elements,
each of them size bytes long, and initializes all its bits to zero.
*/
 void *calloc(size_t nmemb, size_t size)
 {

   size_t size_total = size*nmemb;
   num_requested += size_total;


      if( atexit_registered == 0 )
      {
         atexit_registered = 1;
         atexit( printStatistics );
      }

      /* Align to multiple of 4 */
      size_total = ALIGN4(size_total);

      /* Handle 0 size */
      if (size_total == 0)
      {
         return NULL;
      }

      /* Look for free _block */
      struct _block *last = freeList;
      struct _block *next = findFreeBlock(&last, size_total);

      /* TODO: Split free _block if possible */
      if ( next != NULL)
      {
        if((next->size) > size) // should be > size
        {
          split_block(next, size);
          next->free = false;
        }
        num_reuses++;
      }


      /* Could not find free _block, so grow heap */
      if (next == NULL)
      {
         next = growHeap(last, size_total);
         num_grows++;
         num_blocks++;
      }

      /* Could not find free _block or grow heap, so just return NULL */
      if (next == NULL)
      {
         return NULL;
      }

      /* Mark _block as in use */
      next->free = false;
      //  num_mallocs++;

      memset(BLOCK_DATA(next), 0 , sizeof(size_total));// BLCOK DATA zeroes out
      /* Return data address associated with _block */
      return BLOCK_DATA(next);
 }

void *realloc(void *src, size_t size)
{
  void *dest = malloc(size); // find the new block
  memcpy(dest, src, BLOCK_HEADER(src)->size); // copy all the data
  free(src);// free the original source
  return dest;// return the new address of the new block
}
