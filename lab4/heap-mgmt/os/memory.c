//
//	memory.c
//
//	Routines for dealing with memory management.

// static char rcsid[] = "$Id: memory.c,v 1.1 2000/09/20 01:50:19 elm Exp elm
// $";

#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "memory.h"
#include "queue.h"

// num_pages = size_of_memory / size_of_one_page
static uint32 freemap[MEM_NUM_PAGES / 32];
static uint32 pagestart;
static int nfreepages;
static int freemapmax;

void PrintFreeMap() {
  int i;
  for (i = 0; i < sizeof(freemap) / sizeof(freemap[0]); ++i) {
    printf("%d: 0x%x\n", i, freemap[i]);
  }
}

void MemorySetFreemap(int p, int b) {
  uint32 wd = p / 32;
  uint32 bitnum = p % 32;

  freemap[wd] = (freemap[wd] & invert(1 << bitnum)) | (b << bitnum);
}

//----------------------------------------------------------------------
//
//	This silliness is required because the compiler believes that
//	it can invert a number by subtracting it from zero and subtracting
//	an additional 1.  This works unless you try to negate 0x80000000,
//	which causes an overflow when subtracted from 0.  Simply
//	trying to do an XOR with 0xffffffff results in the same code
//	being emitted.
//
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//
//	MemoryGetSize
//
//	Return the total size of memory in the simulator.  This is
//	available by reading a special location.
//
//----------------------------------------------------------------------
int MemoryGetSize() { return (*((int *)DLX_MEMSIZE_ADDRESS)); }

//----------------------------------------------------------------------
//
//	MemoryModuleInit
//
//	Initialize the memory module of the operating system.
//      Basically just need to setup the freemap for pages, and mark
//      the ones in use by the operating system as "VALID", and mark
//      all the rest as not in use.
//
//----------------------------------------------------------------------

void MemoryModuleInit() {
  int i;
  int curpage;
  int maxpage = MemoryGetSize() / MEM_PAGE_SIZE;
  dbprintf('m', "MemoryModuleInit\n");
  dbprintf('m', "max page: %d\n", maxpage);
  dbprintf('m', "lasaddress: 0x%x , inverted: 0x%x\n", lastosaddress,
           (lastosaddress & invert(0x3)));

  pagestart = ((lastosaddress & invert(0x3)) / MEM_PAGE_SIZE) + 1;
  freemapmax = (maxpage + 31) / 32;
  dbprintf('m', "Map has %d entries, memory size is 0x%x (%d).\n", freemapmax,
           maxpage, maxpage);
  dbprintf('m', "Free pages start with page # 0x%x (%d).\n", pagestart,
           pagestart);
  for (i = 0; i < freemapmax; i++) {
    // Initially, all pages are considered in use.  This is done to make
    // sure we don't have any partially initialized freemap entries.
    freemap[i] = 0;
  }
  nfreepages = 0;
  for (curpage = pagestart; curpage < maxpage; curpage++) {
    nfreepages += 1;
    MemorySetFreemap(curpage, 1);
  }
  dbprintf('m', "Initialized %d free pages.\n", nfreepages);
}

void PrintSysStack(PCB *pcb) {
  int i;
  printf("savedFrame: 0x%x, sysStackPtr: 0x%x\n", pcb->currentSavedFrame,
         pcb->sysStackPtr);
  for (i = 0; i < sizeof(pcb->pagetable) / sizeof(pcb->pagetable[0]); ++i) {
    printf("page %03d: 0x%x\n", i, pcb->pagetable[i]);
  }
}

//----------------------------------------------------------------------
//
// MemoryTranslateUserToSystem
//
//	Translate a user address (in the process referenced by pcb)
//	into an OS (physical) address.  Return the physical address.
//
//----------------------------------------------------------------------
uint32 MemoryTranslateUserToSystem(PCB *pcb, uint32 addr) {
  int page = ADDRESS_TO_PAGE(addr);
  int offset = ADDRESS_TO_OFFSET(addr);
  dbprintf('m',
           "MemoryTranslateUserToSystem: addr: 0x%x, page: %d, offset 0x%x\n",
           addr, page, offset);
  if (pcb->pagetable[page] & MEM_PTE_VALID) {
    dbprintf('m', "Returning 0x%x\n",
             ((pcb->pagetable[page] & MEM_PTE_MASK) | offset));
    return ((pcb->pagetable[page] & MEM_PTE_MASK) | offset);
  }
  dbprintf('m', "MemoryTranslateUserToSystem: failed due to being invalid \n");
  return MEM_FAIL;
}

//----------------------------------------------------------------------
//
//	MemoryMoveBetweenSpaces
//
//	Copy data between user and system spaces.  This is done page by
//	page by:
//	* Translating the user address into system space.
//	* Copying all of the data in that page
//	* Repeating until all of the data is copied.
//	A positive direction means the copy goes from system to user
//	space; negative direction means the copy goes from user to system
//	space.
//
//	This routine returns the number of bytes copied.  Note that this
//	may be less than the number requested if there were unmapped pages
//	in the user range.  If this happens, the copy stops at the
//	first unmapped address.
//
//----------------------------------------------------------------------
int MemoryMoveBetweenSpaces(PCB *pcb, unsigned char *system,
                            unsigned char *user, int n, int dir) {
  unsigned char *curUser;  // Holds current physical address representing
                           // user-space virtual address
  int bytesCopied = 0;     // Running counter
  int bytesToCopy;  // Used to compute number of bytes left in page to be copied

  while (n > 0) {
    // Translate current user page to system address.  If this fails, return
    // the number of bytes copied so far.
    curUser = (unsigned char *)MemoryTranslateUserToSystem(pcb, (uint32)user);

    // If we could not translate address, exit now
    if (curUser == MEM_FAIL) break;

    // Calculate the number of bytes to copy this time.  If we have more bytes
    // to copy than there are left in the current page, we'll have to just copy
    // to the
    // end of the page and then go through the loop again with the next page.
    // In other words, "bytesToCopy" is the minimum of the bytes left on this
    // page
    // and the total number of bytes left to copy ("n").

    // First, compute number of bytes left in this page.  This is just
    // the total size of a page minus the current offset part of the physical
    // address.  MEM_PAGESIZE should be the size (in bytes) of 1 page of memory.
    // MEM_ADDRESS_OFFSET_MASK should be the bit mask required to get just the
    // "offset" portion of an address.
    bytesToCopy = MEM_PAGE_SIZE - ((uint32)curUser & MEM_ADDRESS_OFFSET_MASK);

    // Now find minimum of bytes in this page vs. total bytes left to copy
    if (bytesToCopy > n) {
      bytesToCopy = n;
    }

    // Perform the copy.
    if (dir >= 0) {
      bcopy(system, curUser, bytesToCopy);
    } else {
      bcopy(curUser, system, bytesToCopy);
    }

    // Keep track of bytes copied and adjust addresses appropriately.
    n -= bytesToCopy;            // Total number of bytes left to copy
    bytesCopied += bytesToCopy;  // Total number of bytes copied thus far
    system += bytesToCopy;       // Current address in system space to copy next
                                 // bytes from/into
    user += bytesToCopy;  // Current virtual address in user space to copy next
                          // bytes from/into
  }
  return (bytesCopied);
}

//----------------------------------------------------------------------
//
//	These two routines copy data between user and system spaces.
//	They call a common routine to do the copying; the only difference
//	between the calls is the actual call to do the copying.  Everything
//	else is identical.
//
//----------------------------------------------------------------------
int MemoryCopySystemToUser(PCB *pcb, unsigned char *from, unsigned char *to,
                           int n) {
  return (MemoryMoveBetweenSpaces(pcb, from, to, n, 1));
}

int MemoryCopyUserToSystem(PCB *pcb, unsigned char *from, unsigned char *to,
                           int n) {
  return (MemoryMoveBetweenSpaces(pcb, to, from, n, -1));
}

int AddPageToProcessOrKill(PCB *pcb, int page_idx) {
  uint32 new_page;
  new_page = MemoryAllocPage();
  if (new_page == 0) {
    dbprintf('m', "PID %d. Could not allocate page in AddPageToProcess.\n",
             GetPidFromAddress(pcb));
    ProcessKill();
    return MEM_FAIL;
  }
  pcb->pagetable[page_idx] = MemorySetupPte(new_page);
  pcb->npages += 1;
  return MEM_SUCCESS;
}

//---------------------------------------------------------------------
// MemoryPageFaultHandler is called in traps.c whenever a page fault
// (better known as a "seg fault" occurs.  If the address that was
// being accessed is on the stack, we need to allocate a new page
// for the stack.  If it is not on the stack, then this is a legitimate
// seg fault and we should kill the process.  Returns MEM_SUCCESS
// on success, and kills the current process on failure.  Note that
// fault_address is the beginning of the page of the virtual address that
// caused the page fault, i.e. it is the vaddr with the offset zero-ed
// out.
//
// Note: The existing code is incomplete and only for reference.
// Feel free to edit.
//---------------------------------------------------------------------
int MemoryPageFaultHandler(PCB *pcb) {
  uint32 accessed_addr = pcb->currentSavedFrame[PROCESS_STACK_FAULT];
  uint32 user_stack_ptr =
      pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER];
  dbprintf('m', "MemoryPageFaultHandler\n");
  printf("MemoryPageFaultHandler\n");
  // TODO: (nhendy) do we kill process if there are no available pages?
  if (accessed_addr >= user_stack_ptr) {
    return AddPageToProcessOrKill(pcb, ADDRESS_TO_PAGE(accessed_addr));
  } else {
    printf("PID %d: Killing process due to segfault\n", GetPidFromAddress(pcb));
    ProcessKill();
  }

  return MEM_FAIL;
}

//---------------------------------------------------------------------
// You may need to implement the following functions and access them from
// process.c
// Feel free to edit/remove them
//---------------------------------------------------------------------

inline int MemoryAllocPage() {
  static int mapnum = 0;
  int bitnum;
  uint32 v;

  if (nfreepages == 0) {
    return (0);
  }
  dbprintf('m', "Allocating memory, starting with page %d\n", mapnum);
  while (freemap[mapnum] == 0) {
    mapnum += 1;
    if (mapnum >= freemapmax) {
      mapnum = 0;
    }
  }
  v = freemap[mapnum];
  for (bitnum = 0; (v & (1 << bitnum)) == 0; bitnum++) {
  }
  freemap[mapnum] &= invert(1 << bitnum);
  v = (mapnum * 32) + bitnum;
  dbprintf('m', "Allocated memory, from map %d, page %d, map=0x%x.\n", mapnum,
           v, freemap[mapnum]);
  nfreepages -= 1;
  return (v);
}

void MemoryFreePte(uint32 pte) {
  dbprintf('m', "Freeing Pte %d, addr: 0x%x, pte 0x%x. PID %d", pte,
           ADDRESS_TO_PAGE((pte & MEM_PTE_MASK)), pte, GetCurrentPid());
  MemoryFreePage(ADDRESS_TO_PAGE((pte & MEM_PTE_MASK)));
}

uint32 MemorySetupPte(uint32 page) {
  return ((page * MEM_PAGE_SIZE) | MEM_PTE_VALID);
}

void MemoryFreePage(uint32 page) {
  MemorySetFreemap(page, 1);
  nfreepages += 1;
  dbprintf('m', "Freed page 0x%x, %d remaining.\n", page, nfreepages);
}

int HeapBlockSize(int order) { return pow(2, order) * HEAP_BLOCK_SIZE; }

int HeapAllocateBlockUtil(PCB *pcb, int order, int idx) {
  int result;
  if (pcb->heap[idx].inuse == 1 || pcb->heap[idx].allocated == 1) {
    return MEM_FAIL;
  }
  if (pcb->heap[idx].order == order && pcb->heap[idx].inuse == 0 &&
      pcb->heap[idx].allocated == 0) {
    return idx;
  }
  if (pcb->heap[idx].inuse == 0) {
    printf(
        "Created a left child node (order = %d, addr = %d, size = %d) of "
        "parent (order = %d, addr %d, size = %d)\n",
        pcb->heap[idx].order + 1,
        pcb->heap[GetHeapLeftChild(idx, HEAP_NUM_NODES)].start_address,
        HeapBlockSize(pcb->heap[idx].order + 1), pcb->heap[idx].order,
        pcb->heap[idx].start_address, HeapBlockSize(pcb->heap[idx].order));
  }
  result =
      HeapAllocateBlockUtil(pcb, order, GetHeapLeftChild(idx, HEAP_NUM_NODES));
  if (result != MEM_FAIL) {
    pcb->heap[idx].inuse = 1;
    return result;
  }
  if (pcb->heap[idx].inuse == 0) {
    printf(
        "Created a right child node (order = %d, addr = %d, size = %d) of "
        "parent (order = %d, addr %d, size = %d)\n",
        pcb->heap[idx].order + 1,
        pcb->heap[GetHeapRightChild(idx, HEAP_NUM_NODES)].start_address,
        HeapBlockSize(pcb->heap[idx].order + 1), pcb->heap[idx].order,
        pcb->heap[idx].start_address, HeapBlockSize(pcb->heap[idx].order));
  }
  pcb->heap[idx].inuse = 1;
  return HeapAllocateBlockUtil(pcb, order,
                               GetHeapRightChild(idx, HEAP_NUM_NODES));
}

int HeapAllocateBlock(PCB *pcb, int order) {
  return HeapAllocateBlockUtil(pcb, order, 0);
}

void *malloc(PCB *pcb, int size) {
  int order;
  int node_idx;
  while (pow(2, order) * HEAP_BLOCK_SIZE < size) order++;

  if ((node_idx = HeapAllocateBlock(pcb, order)) == MEM_FAIL) {
    printf("Failed to allocate a block\n");
    return NULL;
  }

  printf(
      "Allocated the block: order = %d, addr = %d, requested mem size = %d, "
      "block size = %d\n",
      pcb->heap[node_idx].order, pcb->heap[node_idx].start_address, size,
      HeapBlockSize(pcb->heap[node_idx].order));

  return (void *)pcb->heap[node_idx].start_address;
}

int SanityCheckHeapAddress(PCB *pcb, void *ptr) {
  return ptr != NULL && ptr >= pcb->heap[0].start_address &&
         ptr <= pcb->heap[HEAP_NUM_NODES - 1].start_address;
}

void HeapMaybeCoalesceBuddies(PCB *pcb, int idx) {
  int parent_idx, right_idx, left_idx;
  int left_address, right_address, child_order;
  if (idx == 0) return;
  if (pcb == NULL) return;
  if (pcb->heap[idx].allocated == 1 || pcb->heap[idx].inuse == 1) return;

  parent_idx = GetHeapParent(idx, HEAP_NUM_NODES);
  right_idx = GetHeapRightChild(parent_idx, HEAP_NUM_NODES);
  left_idx = GetHeapLeftChild(parent_idx, HEAP_NUM_NODES);
  if (pcb->heap[right_idx].allocated == 0 &&
      pcb->heap[left_idx].allocated == 0) {
    left_address = pcb->heap[left_address].start_address;
    right_address = pcb->heap[right_address].start_address;
    child_order = pcb->heap[right_address].order;
    printf(
        "Coalesced buddy nodes (order = %d, addr = %d, size = %d) & (order = "
        "%d, addr = %d, size = %d) into the parent node (order = %d, addr = "
        "%d, size = %d)\n",
        child_order, left_address, HeapBlockSize(child_order), child_order,
        right_address, HeapBlockSize(child_order), pcb->heap[parent_idx].order,
        pcb->heap[parent_idx].start_address,
        HeapBlockSize(pcb->heap[parent_idx].order));
    pcb->heap[parent_idx].inuse = 0;
    HeapMaybeCoalesceBuddies(pcb, parent_idx);
  }
}

void *mfree(PCB *pcb, void *ptr) {
  int i;
  if (!SanityCheckHeapAddress(pcb, ptr)) return MEM_FAIL;
  for (i = 0; i < HEAP_NUM_NODES; ++i) {
    if (pcb->heap[i].inuse == 0 && pcb->heap[i].allocated == 1 &&
        pcb->heap[i].start_address == ptr)
      break;
  }
  if (i == HEAP_NUM_NODES) return MEM_FAIL;

  pcb->heap[i].allocated = 0;
  HeapMaybeCoalesceBuddies(pcb, i);
  printf(
      "Freeing heap block of size %d bytes: virtual address %d, "
      "physical address %d.\n",
      HeapBlockSize(pcb->heap[i].order), pcb->heap[i].start_address,
      MemoryTranslateUserToSystem(pcb, pcb->heap[i].start_address));
  return MEM_FAIL;
}
