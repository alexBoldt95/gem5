/**
 * @file
 * Definitions of a LIFO tag store.
 */

#include "mem/cache/tags/lifo.hh"

#include "debug/CacheRepl.hh"
#include "mem/cache/base.hh"

LIFO::LIFO(const Params *p)
    : BaseSetAssoc(p)
{
}

CacheBlk*
LIFO::accessBlock(Addr addr, bool is_secure, Cycles &lat, int master_id)
{
    CacheBlk *blk = BaseSetAssoc::accessBlock(addr, is_secure, lat, master_id);

    if (blk != nullptr) {
        //do not do anything on
        //an access except for returning the block pointer
        DPRINTF(CacheRepl, "set %x: not moving blk %x (%s) in LIFO ordering\n",
                blk->set, regenerateBlkAddr(blk->tag, blk->set),
                is_secure ? "s" : "ns");
    }

    return blk;
}

CacheBlk*
LIFO::findVictim(Addr addr)
{
    int set = extractSet(addr);
    // grab a replacement candidate
    //from the back of the list
    BlkType *blk = nullptr;
    for (int i = assoc - 1; i >= 0; i--) {
        BlkType *b = sets[set].blks[i];
        if (b->way < allocAssoc) {
            blk = b;
            break;
        }
    }
    assert(!blk || blk->way < allocAssoc);

    if (blk && blk->isValid()) {
        DPRINTF(CacheRepl, "set %x: selecting blk %x for replacement\n",
                set, regenerateBlkAddr(blk->tag, set));
    }

    return blk;
}

void
LIFO::insertBlock(PacketPtr pkt, BlkType *blk)
{
    BaseSetAssoc::insertBlock(pkt, blk);

    int set = extractSet(pkt->getAddr());
    sets[set].moveToTail(blk); //block should be inserted at tail
}

void
LIFO::invalidate(CacheBlk *blk)
{
    BaseSetAssoc::invalidate(blk);

    // should be evicted before valid blocks
    //just invalidate, do not move
}

LIFO*
LIFOParams::create()
{
    return new LIFO(this);
}
