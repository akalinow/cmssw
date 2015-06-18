#ifndef __l1t_regional_muon_candidatefwd_h__
#define __l1t_regional_muon_candidatefwd_h__

#include <vector>

namespace l1t {
	enum tftype {
  	bmtf, omtf_neg, omtf_pos, emtf_neg, emtf_pos
	};
	class L1TRegionalMuonCandidate;
	typedef std::vector<L1TRegionalMuonCandidate> L1TRegionalMuonCandidateCollection;
}

#endif /* define __l1t_regional_muon_candidatefwd_h__ */