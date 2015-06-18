#ifndef __l1t_gmt_internal_muonfwd_h__
#define __l1t_gmt_internal_muonfwd_h__

namespace l1t {
  class L1TGMTInternalMuon;
  typedef std::vector<L1TGMTInternalMuon> L1TGMTInternalMuonCollection;
  typedef std::map<int, std::vector<std::shared_ptr<L1TGMTInternalMuon>>> L1TGMTInternalWedges;
  typedef std::list<std::shared_ptr<L1TGMTInternalMuon>> L1TGMTInternalMuonList;

}

#endif /* define __l1t_gmt_internal_muon_h__ */