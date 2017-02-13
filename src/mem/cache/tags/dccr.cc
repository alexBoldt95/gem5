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
    : BaseSetAssoc(p)
{
  num_policies = 4;
  historyTableNewest = nullptr;
  historyTableOldest = nullptr;
  historyTableSize = 0;
  maxHistoryTableSize = 256;
  NMRU_uses = 0;
  LFU_uses = 0;
  FIFO_uses = 0;
  LIFO_uses = 0;
  findVictim_calls = 0;
  /*
  //mimic initialization of sets for queue_sets
  queue_sets = new SetType[numSets];
  queue_blks = new BlkType[numSets * assoc];

  queue_dataBlks = new uint8_t[numBlocks * blkSize];

  unsigned blkIndex = 0;       // index into blks array
  for (unsigned i = 0; i < numSets; ++i) {
      queue_sets[i].assoc = assoc;

      queue_sets[i].blks = new BlkType*[assoc];

      // link in the data blocks
      for (unsigned j = 0; j < assoc; ++j) {
          // locate next cache block
          BlkType *blk = &queue_blks[blkIndex];
          blk->data = &queue_dataBlks[blkSize*blkIndex];
          ++blkIndex;

          // invalidate new cache block
          blk->invalidate();

          //EGH Fix Me : do we need to initialize blk?

          // Setting the tag to j is just to prevent long chains in the hash
          // table; won't matter because the block is invalid
          blk->tag = j;
          blk->whenReady = 0;
          blk->isTouched = false;
          blk->size = blkSize;
          queue_sets[i].blks[j]=blk;
          blk->set = i;
          blk->way = j;
      }
  }
  */
}

CacheBlk*
DCCR::accessBlock(Addr addr, bool is_secure, Cycles &lat, int master_id)
{
    CacheBlk *blk = BaseSetAssoc::accessBlock(addr, is_secure, lat, master_id);

    if (blk != nullptr) {
      CacheBlk* temp;
        //do not do anything on
        //an access except for returning the block pointer
        //wipe mruBit
        for (int i = 0; i < assoc; i++){
          assert(i < assoc);
          assert(i >= 0);
          temp= sets[extractSet(addr)].blks[i];
          temp->mruBit = 0;
        }
        //set mruBit
        blk->mruBit = 1;

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
            stat_scores[i] += (maxHistoryTableSize - evictedInfo->value);
          }
        }
      }

    }
    return blk;
}

CacheBlk*
DCCR::NMRU_findVictim(Addr addr){
  BlkType *blk = BaseSetAssoc::findVictim(addr);
  if (blk->isValid()){
    int set = extractSet(addr);
    // grab a replacement candidate

    BlkType *blk = nullptr;
    for (int i = 0; i < assoc; i++) {
        BlkType *b = sets[set].blks[i];
        if (b->mruBit == 0) {
            blk = b;
            break;
        }
    }
    assert(!blk || blk->way < allocAssoc);

    if (blk && blk->isValid()) {
        DPRINTF(CacheRepl, "set %x: selecting NMRU blk %x for replacement\n",
                set, regenerateBlkAddr(blk->tag, set));
    }
  }
  return blk;
}

CacheBlk*
DCCR::LFU_findVictim(Addr addr){
  BlkType *blk = BaseSetAssoc::findVictim(addr);
  BlkType* minBlk = blk;
  //find the blk with the least refCount

  if (blk->isValid()){
    int curr_min_refs = std::numeric_limits<int>::max();
    for (int i = 0; i < assoc; i++){
      assert(i < assoc);
      assert(i >= 0);
      blk = sets[extractSet(addr)].blks[i];
      if ((blk->refCount < curr_min_refs)
          && blk->isValid()){
        curr_min_refs = blk->refCount;
        minBlk = blk;
      }
    }
    assert(minBlk);
    DPRINTF(CacheRepl, "set %x: selecting LFU blk %x for replacement\n",
            minBlk->set, regenerateBlkAddr(minBlk->tag, minBlk->set));
  }
  return minBlk;
}

CacheBlk*
DCCR::FIFO_findVictim(Addr addr){
  BlkType *blk = BaseSetAssoc::findVictim(addr);
  if (blk->isValid()){
    int set = extractSet(addr);
    // grab a replacement candidate
    BlkType *blk = sets[set].blks[0]; //get the front of the list
    assert(!blk || blk->way < allocAssoc);

    if (blk && blk->isValid()) {
        DPRINTF(CacheRepl, "set %x: selecting FIFO blk %x for replacement\n",
                set, regenerateBlkAddr(blk->tag, set));
    }
 }
  return blk;
}

CacheBlk*
DCCR::LIFO_findVictim(Addr addr){
  BlkType *blk = BaseSetAssoc::findVictim(addr);
  if (blk->isValid()){
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
        DPRINTF(CacheRepl, "set %x: selecting LIFO blk %x for replacement\n",
                set, regenerateBlkAddr(blk->tag, set));
    }
  }
  return blk;
}



CacheBlk*
DCCR::findVictim(Addr addr)
{
  findVictim_calls++;
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

  choices[0] = NMRU_findVictim(addr);
  choices[1] = LFU_findVictim(addr);
  choices[2] = FIFO_findVictim(addr);
  choices[3] = LIFO_findVictim(addr);

/*
  choices[0] = sets[set].blks[0];
  choices[1] = sets[set].blks[0];
  choices[2] = sets[set].blks[0];
  choices[3] = sets[set].blks[0];
*/
  BlkType* repl_blk = choices[chosen_policy_idx];
  if (repl_blk != nullptr){
    if (chosen_policy_idx == 0){
      NMRU_uses++;
    } else if (chosen_policy_idx == 1){
      LFU_uses++;
    } else if (chosen_policy_idx == 2){
      FIFO_uses++;
    } else if (chosen_policy_idx == 3){
      LIFO_uses++;
    }
    Block_ID_t repl_blk_ID = (Block_ID_t) getBlockID(repl_blk);
    addToHistoryTable(repl_blk, choices, repl_blk_ID);
  }

  if (repl_blk && repl_blk->isValid()) {
      DPRINTF(CacheRepl, "set in DCCR %x: selecting blk %x for replacement\n",
              set, regenerateBlkAddr(repl_blk->tag, set));
  }

  return repl_blk;
}
/*
void
DCCR::NMRU_insertBlock(PacketPtr pkt, BlkType *blk){
  int set = extractSet(pkt->getAddr());
  sets[set].moveToHead(blk);
}

void
DCCR::FIFO_insertBlock(PacketPtr pkt, BlkType *blk){
  int set = extractSet(pkt->getAddr());
  sets[set].moveToTail(blk); //block should be inserted at tail
}
*/
void
DCCR::insertBlock(PacketPtr pkt, BlkType *blk)
{
    BaseSetAssoc::insertBlock(pkt, blk);
    int set = extractSet(pkt->getAddr());
    blk->mruBit = 1;
    sets[set].moveToTail(blk); //block should be inserted at tail
}

void
DCCR::invalidate(CacheBlk *blk)
{
    BaseSetAssoc::invalidate(blk);
    // should be evicted before valid blocks
    int set = blk->set;
    blk->mruBit = 0;
    sets[set].moveToTail(blk);
    //just invalidate, do not move
}

DCCR*
DCCRParams::create()
{
    return new DCCR(this);
}

void
DCCR::regStats(){

  BaseSetAssoc::regStats();

  using namespace Stats;

  NMRU_uses
      .name(name() + ".NMRU_uses")
      .desc("number of times NMRU is used to evict")
      .flags(none)
      ;

  LFU_uses
      .name(name() + ".LFU_uses")
      .desc("number of times LFU is used to evict")
      .flags(none)
      ;

  FIFO_uses
      .name(name() + ".FIFO_uses")
      .desc("number of times FIFO is used to evict")
      .flags(none)
      ;

  LIFO_uses
      .name(name() + ".LIFO_uses")
      .desc("number of times LIFO is used to evict")
      .flags(none)
      ;

  findVictim_calls
      .name(name() + ".findVictim_calls")
      .desc("number of times findVictim is called")
      .flags(none)
      ;

    stat_scores.init(num_policies);
    stat_scores
      .name(name() + ".scores_array")
      .desc("score array after end of run [NMRU, LFU, FIFO, LIFO]")
      .flags(none)
      ;

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
