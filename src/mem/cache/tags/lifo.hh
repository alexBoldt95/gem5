#ifndef __MEM_CACHE_TAGS_LIFO_HH__
#define __MEM_CACHE_TAGS_LIFO_HH__

#include "mem/cache/tags/base_set_assoc.hh"
#include "params/LIFO.hh"

class LIFO : public BaseSetAssoc
{
  public:
    /** Convenience typedef. */
    typedef LIFOParams Params;

    /**
     * Construct and initialize this tag store.
     */
    LIFO(const Params *p);

    /**
     * Destructor
     */
    ~LIFO() {}

    /**
     * Required functions for this subclass to implement
     */
    BlkType* accessBlock(Addr addr, bool is_secure, Cycles &lat,
                         int context_src);
    BlkType* findVictim(Addr addr);
    void insertBlock(PacketPtr pkt, BlkType *blk);
    void invalidate(BlkType *blk);
};

#endif // __MEM_CACHE_TAGS_LIFO_HH__
