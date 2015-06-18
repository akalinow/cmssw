#include "DataFormats/L1TMuon/interface/L1TGMTInternalMuon.h"

namespace l1t {

L1TGMTInternalMuon::L1TGMTInternalMuon(const L1TGMTInternalMuon& other) :
  m_regional(other.m_regional), m_hwRank(other.m_hwRank), m_hwCancelBit(other.m_hwCancelBit), m_hwIsoSum(other.m_hwIsoSum), 
  m_hwDeltaEta(other.m_hwDeltaEta), m_hwDeltaPhi(other.m_hwDeltaPhi), m_hwAbsIso(other.m_hwAbsIso), m_hwRelIso(other.m_hwRelIso), 
  m_hwGlobalPhi(other.m_hwGlobalPhi), m_hwCaloIndex(other.m_hwCaloIndex) {

}

L1TGMTInternalMuon::L1TGMTInternalMuon(const L1TRegionalMuonCandidate& regional) : 
	m_regional(&regional), m_hwRank(0), m_hwCancelBit(0), m_hwIsoSum(0), m_hwDeltaEta(0), m_hwDeltaPhi(0), m_hwAbsIso(-1), m_hwRelIso(-1), m_hwGlobalPhi(-1)
{
  if (m_regional->trackFinderType() == bmtf) {
      // each BMTF processor corresponds to a 30 degree wedge = 48 in int-scale
      m_hwGlobalPhi = (m_regional->processor() - 1) * 48 + m_regional->hwPhi();
      // first processor starts at CMS phi = -15 degrees...
      m_hwGlobalPhi += 576-24;
      // handle wrap-around (since we add the 576-24, the value will never be negative!)
      m_hwGlobalPhi = m_hwGlobalPhi%576;
  } else {
      // all others correspond to 60 degree sectors = 96 in int-scale
      m_hwGlobalPhi = (m_regional->processor() - 1) * 96 + m_regional->hwPhi();
      // first processor starts at CMS phi = 15 degrees... Handle wrap-around with %:
      m_hwGlobalPhi = (m_hwGlobalPhi + 24) % 576;
  }
}

void
L1TGMTInternalMuon::setExtrapolation(int deta, int dphi)
{ 
  m_hwDeltaEta = deta; 
  m_hwDeltaPhi = dphi; 
}

} // namespace l1t