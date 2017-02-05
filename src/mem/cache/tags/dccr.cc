/**
 * @file
 * Definitions of a DCCR tag store.
 */

#include "mem/cache/tags/dccr.hh"

#include "debug/CacheRepl.hh"
#include "mem/cache/base.hh"
#include "mem/cache/tags/fifo.hh"
#include "mem/cache/tags/lfu.hh"
#include "mem/cache/tags/lifo.hh"
#include "mem/cache/tags/lru.hh"





DCCR::DCCR(const Params *p)
    : BaseSetAssoc(p),
    lru_tag(p),
    lfu_tag(p),
    fifo_tag(p),
    lifo_tag(p)
{
  //scores = {0};
  num_policies = 4;
}

CacheBlk*
DCCR::accessBlock(Addr addr, bool is_secure, Cycles &lat, int master_id)
{
    CacheBlk *blk = BaseSetAssoc::accessBlock(addr, is_secure, lat, master_id);

    if (blk != nullptr) {
        //do not do anything on
        //an access except for returning the block pointer
        DPRINTF(CacheRepl, "set %x: not moving blk %x (%s) in DCCR ordering\n",
                blk->set, regenerateBlkAddr(blk->tag, blk->set),
                is_secure ? "s" : "ns");
    } else{
      //miss, punish policies
      Block_ID_t blkID = (Block_ID_t) getBlockID(addr);
      victimInfo* evictedInfo = searchHistoryTable(blkID);
      if (evictedInfo != NULL) {
        //set valid bit to zero
        evictedInfo->validBit = 0;

        //update each policies score
        //we need to calculate what value to add to each score
        for (int i = 0; i < num_policies; i++) {
          if (evictedInfo->choseToEvict[i] != 0) {
            scores[i] += (maxHistoryTableSize - evictedInfo->value);
          }
        }
      }

    }
    return blk;
}

unsigned int
DCCR::getBlockID(Addr addr){
  unsigned int set = (unsigned int) extractSet(addr);
  unsigned int tag = (unsigned int) extractTag(addr);
  unsigned int ret = ((tag) << (tagShift - setShift));
  ret = ret | set;
  return ret;
}

unsigned int
DCCR::getBlockID(BlkType* blk){
  unsigned int set = (unsigned int) blk->set;
  unsigned int tag = (unsigned int) blk->tag;
  unsigned int ret = ((tag) << (tagShift - setShift));
  ret = ret | set;
  return ret;
}

CacheBlk*
DCCR::findVictim(Addr addr)
{
  int set = extractSet(addr);

  BlkType* choices[num_policies];
  unsigned int lowest_score = 0xffffffff;
  unsigned int chosen_policy_idx = 0;
  for (int i = 0; i < num_policies; i++) {
    if (scores[i] < lowest_score) {
      lowest_score = scores[i];
      chosen_policy_idx = i;
    }
  }

  choices[0] = lru_tag.findVictim(addr);
  choices[1] = lfu_tag.findVictim(addr);
  choices[2] = fifo_tag.findVictim(addr);
  choices[3] = lifo_tag.findVictim(addr);

  BlkType* repl_blk = choices[chosen_policy_idx];
  Block_ID_t repl_blk_ID = (Block_ID_t) getBlockID(repl_blk);
  addToHistoryTable(repl_blk, choices, repl_blk_ID);

  if (repl_blk && repl_blk->isValid()) {
      DPRINTF(CacheRepl, "set in DCCR %x: selecting blk %x for replacement\n",
              set, regenerateBlkAddr(repl_blk->tag, set));
  }

  return repl_blk;
}

void
DCCR::insertBlock(PacketPtr pkt, BlkType *blk)
{
    BaseSetAssoc::insertBlock(pkt, blk);

    int set = extractSet(pkt->getAddr());
    sets[set].moveToTail(blk); //block should be inserted at tail
}

void
DCCR::invalidate(CacheBlk *blk)
{
    BaseSetAssoc::invalidate(blk);

    // should be evicted before valid blocks
    //just invalidate, do not move
}

DCCR*
DCCRParams::create()
{
    return new DCCR(this);
}

victimInfo*
DCCR::searchHistoryTable (Block_ID_t blkID) {
  //
  struct victimInfo* current = historyTableNewest;
  while (current != NULL){
    if (current->blockID == blkID){
      return current;
    }
    current->value += 1;
    current = current->next;
  }
  return NULL;
}

void
DCCR::addToHistoryTable(BlkType* blk, BlkType** choice_arr, Block_ID_t blkID) {
  struct victimInfo * newVictim = (struct victimInfo*)
   malloc(sizeof(struct victimInfo));
  newVictim->blockID = blkID;
  newVictim->validBit = 1;
  newVictim->value = 0;
  for (int i = 0; i < 4; i++) {
    if (choice_arr[i] == blk){
      newVictim->choseToEvict[i] = 1;
    } else {
      newVictim->choseToEvict[i] = 0;
    }
  }
    if (historyTableSize == 0){
      historyTableNewest = newVictim;
      historyTableOldest = newVictim;
      newVictim->next = NULL;
      newVictim->prev = NULL;
    }else{
      newVictim->next = historyTableNewest;
      historyTableNewest->prev = newVictim;
      historyTableNewest = newVictim;
      newVictim->prev = NULL;
    }

    if (historyTableSize < maxHistoryTableSize) {
      historyTableSize++;
    } else {
      historyTableOldest = historyTableOldest->prev;
      free(historyTableOldest->next);
      historyTableOldest->next = NULL;
    }
}
