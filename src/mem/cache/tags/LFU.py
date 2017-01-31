from Tags import BaseSetAssoc

class LFU(BaseSetAssoc):
    type = "LFU"
    cxx_header = "mem/cache/tags/lfu.hh"
