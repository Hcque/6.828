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

#define SIZE 13
struct {
  struct spinlock lock[SIZE];
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  struct buf head[SIZE];
} bcache;

void
binit(void)
{
  struct buf *b;


  // Create linked list of buffers
  for (int i = 0; i < SIZE; i++){
  initlock(&bcache.lock[i], "bcache");
    bcache.head[i].prev = &bcache.head[i];
    bcache.head[i].next = &bcache.head[i];
  }

  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    int hvalue = b->blockno % SIZE;
    b->next = bcache.head[hvalue].next;
    b->prev = &bcache.head[hvalue];
    initsleeplock(&b->lock, "buffer");
    bcache.head[hvalue].next->prev = b;
    bcache.head[hvalue].next = b;
  }

}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  struct buf *min;

  int hvalue = blockno % SIZE;
  min = bcache.head[hvalue].next;

  acquire(&bcache.lock[hvalue]);
  // struct buf bucket = bcache.head[hvalue];

  // Is the block already cached?
  for(b = bcache.head[hvalue].next; b != &bcache.head[hvalue]; b = b->next){
    // printf("refcnt:%d\n",b->refcnt);
    if(b->ticks < min->ticks && b->refcnt == 0){
      min = b;
    }

    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.lock[hvalue]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached. using timestamps
  // Recycle the least recently used (LRU) unused buffer.

  // do the eviction


  // for(b = bcache.head.prev; b != &bcache.head; b = b->prev){
    b = min;
    printf("ref:%d\n", b->refcnt);
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache.lock[hvalue]);
      acquiresleep(&b->lock);
      return b;
    }

  // acquire(&bcache.)
  // find in other hash buckets
  for (int i = 0 ; i < SIZE; i++){
    if (i != hvalue){

    acquire(&bcache.lock[i]);
    struct buf *hea = &bcache.head[i];
    for (b = hea->next; b != hea; b = b->next){
      if (b->refcnt == 0){
        b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;

      b->prev->next = b->next;
      b->next->prev = b->prev;
      release(&bcache.lock[i]);

      b->prev = &bcache.head[hvalue];
      b->next = bcache.head[hvalue].next;
      bcache.head[hvalue].next->prev = b;
      bcache.head[hvalue].next = b;

      release(&bcache.lock[hvalue]);
      acquiresleep(&b->lock);
      return b;

      }
    }
    release(&bcache.lock[i]);

    }
  }

  
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

  // acquire(&bcache.lock);
  acquire(&bcache.lock[ b->blockno%SIZE ]);
  b->refcnt--;
  if (b->refcnt == 0) {
    b->ticks = ticks;
    // no one is waiting for it.
    // b->next->prev = b->prev;
    // b->prev->next = b->next;
    // b->next = bcache.head.next;
    // b->prev = &bcache.head;
    // bcache.head.next->prev = b;
    // bcache.head.next = b;
  }
  release(&bcache.lock[ b->blockno%SIZE ]);
}

void
bpin(struct buf *b) {
  acquire(&bcache.lock[b->blockno%SIZE]);
  b->refcnt++;
  release(&bcache.lock[b->blockno%SIZE]);
}

void
bunpin(struct buf *b) {
  acquire(&bcache.lock[b->blockno%SIZE]);
  b->refcnt--;
  release(&bcache.lock[b->blockno%SIZE]);
}


