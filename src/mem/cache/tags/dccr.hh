#ifndef __MEM_CACHE_TAGS_DCCR_HH__
#define __MEM_CACHE_TAGS_DCCR_HH__



#include <vector>

#include "base/statistics.hh"
#include "mem/cache/tags/base_set_assoc.hh"
#include "params/DCCR.hh"



struct victimInfo {
  unsigned int blockID;  //Tag + set bits
  char choseToEvict[4];  //array of bits
  //indicating the policies that chose to evict this block
  char validBit;         //bit indicating this
  //block is valid in the vitim table
  struct victimInfo* next; //pointer to next victim in linked list
  struct victimInfo* prev; //pointer to prev
  unsigned int value;     //age
};

class DCCR : public BaseSetAssoc
{
  public:
    /** Convenience typedef. */
    typedef DCCRParams Params;
    typedef unsigned int Block_ID_t;

    unsigned int num_policies;
    unsigned int scores[4] = {0, 0, 0, 0};
    Stats::Vector stat_scores;
    /*
    SetType* queue_sets;

    BlkType* queue_blks;

    uint8_t *queue_dataBlks;
    */
    victimInfo* historyTableOldest;
    victimInfo* historyTableNewest;
    unsigned int historyTableSize;
    unsigned int maxHistoryTableSize;

    /**
     * Construct and initialize this tag store.
     */
    DCCR(const Params *p);

    /**
     * Destructor
     */
    ~DCCR() {}

    /**
     * Required functions for this subclass to implement
     */
    BlkType* accessBlock(Addr addr, bool is_secure, Cycles &lat,
                         int context_src);
    BlkType* findVictim(Addr addr);
    void insertBlock(PacketPtr pkt, BlkType *blk);
    void invalidate(BlkType *blk);
    unsigned int getBlockID(Addr addr);
    unsigned int getBlockID(BlkType* blk);
    victimInfo* searchHistoryTable(Block_ID_t blkID);
    void addToHistoryTable(BlkType* blk,
      BlkType** choice_arr, Block_ID_t blkID);
/*
    BlkType* NMRU_accessBlock(Addr addr, bool is_secure, Cycles &lat,
                         int context_src);
    BlkType* LFU_accessBlock(Addr addr, bool is_secure, Cycles &lat,
                         int context_src);
    BlkType* FIFO_accessBlock(Addr addr, bool is_secure, Cycles &lat,
                         int context_src);
    BlkType* LIFO_accessBlock(Addr addr, bool is_secure, Cycles &lat,
                         int context_src);
*/
    BlkType* NMRU_findVictim(Addr addr);
    BlkType* LFU_findVictim(Addr addr);
    BlkType* FIFO_findVictim(Addr addr);
    BlkType* LIFO_findVictim(Addr addr);

    void NMRU_insertBlock(PacketPtr pkt, BlkType *blk);
    //void LFU_insertBlock(PacketPtr pkt, BlkType *blk);
    void FIFO_insertBlock(PacketPtr pkt, BlkType *blk);
    //void LIFO_insertBlock(PacketPtr pkt, BlkType *blk);
    Stats::Scalar NMRU_uses;
    Stats::Scalar LFU_uses;
    Stats::Scalar FIFO_uses;
    Stats::Scalar LIFO_uses;
    Stats::Scalar findVictim_calls;
/*
    void NMRU_invalidate(BlkType *blk);
    void LFU_invalidate(BlkType *blk);
    void FIFO_invalidate(BlkType *blk);
    void LIFO_invalidate(BlkType *blk);
  */
  virtual void regStats();
};

#endif // __MEM_CACHE_TAGS_DCCR_HH__
