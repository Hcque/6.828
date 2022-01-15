// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define LK_SZ 13

struct {
  struct spinlock lock[LK_SZ]; 
  struct spinlock global_lock; // make the contention problem !!
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf head[LK_SZ];
} bcache;

void
binit(void)
{
  printf("binit\n");
  struct buf *b;

  // initlock(&(bcache.global_lock), "bcache_" + 'G');
  // int each = NBUF/LK_SZ, 
  int c = 0;
  for (int i = 0; i < LK_SZ; i ++ )
  {
    initlock(&(bcache.lock[i]), "bcache_" );

    // Create linked list of buffers
    bcache.head[i].prev = &bcache.head[i];
    bcache.head[i].next = &bcache.head[i];
    

  }
  
  for(b = bcache.buf; b < bcache.buf+NBUF; b++, c++){
      // printf("%d %d ", c, i);
      b->next = bcache.head[0].next;
      b->prev = &bcache.head[0];
      initsleeplock(&b->lock, "buffer");
      bcache.head[0].next->prev = b;
      bcache.head[0].next = b;
      // printf("%d\n", b->refcnt);
    }

  printf("binit DONE\n");
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  uint _idx = blockno%LK_SZ;

  // printf("%d \n",_idx);
  // acquire(&bcache.global_lock);
  acquire(&bcache.lock[_idx]);

  // Is the block already cached?
  for(b = bcache.head[_idx].next; b != &bcache.head[_idx]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[_idx]);
      // release(&bcache.global_lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  for (int j = (_idx+1)%LK_SZ; j != _idx; j=(j+1)%LK_SZ ){

    acquire(&bcache.lock[j]);
    for(b = bcache.head[j].prev; b != &bcache.head[j]; b = b->prev){
      if(b->refcnt == 0) {
        b->dev = dev;
        b->blockno = blockno;
        b->valid = 0;
        b->refcnt = 1;

        b->next->prev = b->prev;
        b->prev->next = b->next;
        release(&bcache.lock[j]);
        b->next = bcache.head[_idx].next;
        b->prev = &bcache.head[_idx];
        bcache.head[_idx].next->prev = b;
        bcache.head[_idx].next = b;

        
        release(&bcache.lock[_idx]);
        // release(&bcache.global_lock);
        acquiresleep(&b->lock);
        return b;
      }
    }
    release(&bcache.lock[j]);

  } // for j
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  int _idx = b->blockno%LK_SZ;
  // acquire(&bcache.global_lock);
  acquire(&bcache.lock[_idx]);
  b->refcnt--;
  if (b->refcnt == 0) {
    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;
    b->next = bcache.head[_idx].next;
    b->prev = &bcache.head[_idx];
    bcache.head[_idx].next->prev = b;
    bcache.head[_idx].next = b;
  }
  
  release(&bcache.lock[_idx]);
  // release(&bcache.global_lock);
}

void
bpin(struct buf *b) {
  int _idx = (b->blockno)%LK_SZ;
  // acquire(&bcache.global_lock);
  acquire(&bcache.lock[_idx]);
  b->refcnt++;
  release(&bcache.lock[_idx]);
  // release(&bcache.global_lock);
}

void
bunpin(struct buf *b) {
  int _idx = (b->blockno)%LK_SZ;
  // acquire(&bcache.global_lock);
  acquire(&bcache.lock[_idx]);
  b->refcnt--;
  release(&bcache.lock[_idx]);
  // release(&bcache.global_lock);
}


