#include "DataFormats/L1TMuon/interface/L1TMuonCandidateTrack.h"

using namespace L1TMuon;

CandidateTrack::
CandidateTrack(const edm::Ref<InternalTrackCollection>& parent):
  L1MuGMTCand(),
  _parent(parent){
}




