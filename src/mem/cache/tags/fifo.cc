/**
 * @file
 * Definitions of a FIFO tag store.
 */

#include "mem/cache/tags/fifo.hh"

#include "debug/CacheRepl.hh"
#include "mem/cache/base.hh"

FIFO::FIFO(const Params *p)
    : BaseSetAssoc(p)
{
}

CacheBlk*
FIFO::accessBlock(Addr addr, bool is_secure, Cycles &lat, int master_id)
{
    CacheBlk *blk = BaseSetAssoc::accessBlock(addr, is_secure, lat, master_id);

    if (blk != nullptr) {
        //do not do anything on an access except
        //for returning the block pointer
        DPRINTF(CacheRepl, "set %x: not moving blk %x (%s) in FIFO ordering\n",
                blk->set, regenerateBlkAddr(blk->tag, blk->set),
                is_secure ? "s" : "ns");
    }

    return blk;
}

CacheBlk*
FIFO::findVictim(Addr addr)
{
    int set = extractSet(addr);
    // grab a replacement candidate
    BlkType *blk = sets[set].blks[0]; //get the front of the list
    assert(!blk || blk->way < allocAssoc);

    if (blk && blk->isValid()) {
        DPRINTF(CacheRepl, "set %x: selecting blk %x for replacement\n",
                set, regenerateBlkAddr(blk->tag, set));
    }

    return blk;
}

void
FIFO::insertBlock(PacketPtr pkt, BlkType *blk)
{
    BaseSetAssoc::insertBlock(pkt, blk);

    int set = extractSet(pkt->getAddr());
    sets[set].moveToTail(blk); //block should be inserted at tail
}

void
FIFO::invalidate(CacheBlk *blk)
{
    BaseSetAssoc::invalidate(blk);

    // should be evicted before valid blocks
    //just invalidate, do not move
}

FIFO*
FIFOParams::create()
{
    return new FIFO(this);
}
