#ifndef FILE_BLOCKALLOC
#define FILE_BLOCKALLOC

/**************************************************************************/
/* File:   blockalloc.hpp                                                 */
/* Author: Joachim Schoeberl                                              */
/* Date:   19. Apr. 2000                                                  */
/**************************************************************************/

namespace ngstd
{

/**
   Optimized memory handler.
   Memory handler allocates many objects at once.
   Maintains free list of deleted objects
 */
class BlockAllocator
{
  /// size of data
  unsigned int size;
  /// number of blocks allocated at once
  unsigned int blocks;
  /// single linked list of free elements
  void * freelist;
  /// pointers to blocks
  Array<char*> bablocks;
  ///
  int nels;
public:
  /// Create BlockAllocator for elements of size asize
  BlockAllocator (unsigned int asize, unsigned int ablocks = 100);
  /// Delete all memeory
  ~BlockAllocator ();

  /// Return pointer to new element
  void * Alloc ();
  /*
  {
    nels++;
    if (!freelist)
      Alloc2 ();

    void * p = freelist;
    freelist = *(void**)freelist;
    return p;
  }
  */


  /// Send memory to free-list
  void Free (void * p);
  /*
  {
    nels--;
    *(void**)p = freelist;
    freelist = p;
  }
  */

  /// number of allocated elements
  int NumElements () { return nels; }

  void Print (ostream * ost) const;
private:
  void * Alloc2 ();
};


}

#endif
