/**
 * @file
 * Definitions of a LFU tag store.
 */
#include <limits>

#include "base/random.hh"
#include "debug/CacheRepl.hh"
#include "mem/cache/base.hh"
#include "mem/cache/tags/lfu.hh"

LFU::LFU(const Params *p)
    : BaseSetAssoc(p)
{
}

BaseSetAssoc::BlkType*
LFU::accessBlock(Addr addr, bool is_secure, Cycles &lat, int master_id)
{
    // Accesses are based on parent class, no need to do anything special
    BlkType *blk = BaseSetAssoc::accessBlock(addr, is_secure, lat, master_id);

    if (blk != NULL) {
        // move this block to head of the MRU list
        sets[blk->set].moveToHead(blk);
        DPRINTF(CacheRepl, "set %x: moving blk %x (%s) to MRU\n",
                blk->set, regenerateBlkAddr(blk->tag, blk->set),
                is_secure ? "s" : "ns");
    }

    return blk;
}



BaseSetAssoc::BlkType*
LFU::findVictim(Addr addr)
{
    BlkType *blk = BaseSetAssoc::findVictim(addr);
    minBlk = NULL;
    //find the blk with the least refCount

    if (blk->isValid()){
      int curr_min_refs = std::numeric_limits<int>::max();
      for (int i = 0; i < assoc; i++){
        assert(idx < assoc);
        assert(idx >= 0);
        blk = sets[extractSet(addr)].blks[i];
        if (blk->refCount < curr_min_refs){
          curr_min_refs = blk->refCount;
          minBlk = blk;
        }
      }
      assert(minBlk);
      DPRINTF(CacheRepl, "set %x: selecting blk %x for replacement\n",
              minBlk->set, regenerateBlkAddr(minBlk->tag, minBlk->set));
    }
    return minBlk;
}

void
LFU::insertBlock(PacketPtr pkt, BlkType *blk)
{
    BaseSetAssoc::insertBlock(pkt, blk);

    int set = extractSet(pkt->getAddr());
    sets[set].moveToHead(blk);
}

void
LFU::invalidate(BlkType *blk)
{
    BaseSetAssoc::invalidate(blk);

    // should be evicted before valid blocks
    int set = blk->set;
    sets[set].moveToTail(blk);
}

LFU*
LFUParams::create()
{
    return new LFU(this);
}
