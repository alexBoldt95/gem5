#ifndef __MEM_CACHE_TAGS_LFU_HH__
#define __MEM_CACHE_TAGS_LFU_HH__

#include "mem/cache/tags/base_set_assoc.hh"
#include "params/LFU.hh"

class LFU : public BaseSetAssoc
{
  public:
    /** Convenience typedef. */
    //typedef LFUParams Params;

    /**
     * Construct and initialize this tag store.
     */
    LFU(const Params *p);

    /**
     * Destructor
     */
    ~LFU() {}

    /**
     * Required functions for this subclass to implement
     */
    BlkType* accessBlock(Addr addr, bool is_secure, Cycles &lat,
                         int context_src);
    BlkType* findVictim(Addr addr);
    void insertBlock(PacketPtr pkt, BlkType *blk);
    void invalidate(BlkType *blk);
};

#endif // __MEM_CACHE_TAGS_LFU_HH__
