#include "../interface/MicroGMTCancelOutUnit.h"
#include "DataFormats/L1TMuon/interface/L1TGMTInternalMuon.h"

namespace l1t {
MicroGMTCancelOutUnit::MicroGMTCancelOutUnit (const edm::ParameterSet& iConfig) : 
    m_boPosMatchQualLUT(iConfig, "BOPos", cancel_t::omtf_bmtf_pos),
    m_boNegMatchQualLUT(iConfig, "BONeg", cancel_t::omtf_bmtf_neg),
    m_foPosMatchQualLUT(iConfig, "FOPos", cancel_t::omtf_emtf_pos),
    m_foNegMatchQualLUT(iConfig, "FONeg", cancel_t::omtf_emtf_neg),
    m_brlSingleMatchQualLUT(iConfig, "BrlSingle", cancel_t::bmtf_bmtf),
    m_ovlPosSingleMatchQualLUT(iConfig, "OvlPosSingle", cancel_t::omtf_omtf_pos),
    m_ovlNegSingleMatchQualLUT(iConfig, "OvlNegSingle", cancel_t::omtf_omtf_neg),
    m_fwdPosSingleMatchQualLUT(iConfig, "FwdPosSingle", cancel_t::emtf_emtf_pos),
    m_fwdNegSingleMatchQualLUT(iConfig, "FwdNegSingle", cancel_t::emtf_emtf_neg)
  {
    m_lutDict[tftype::bmtf+tftype::bmtf*5] = &m_brlSingleMatchQualLUT;
    m_lutDict[tftype::omtf_neg+tftype::bmtf*5] = &m_boNegMatchQualLUT;
    m_lutDict[tftype::omtf_pos+tftype::bmtf*5] = &m_boPosMatchQualLUT;
    m_lutDict[tftype::omtf_pos+tftype::omtf_pos*5] = &m_ovlPosSingleMatchQualLUT;
    m_lutDict[tftype::omtf_neg+tftype::omtf_neg*5] = &m_ovlNegSingleMatchQualLUT;
    m_lutDict[tftype::emtf_pos+tftype::emtf_pos*5] = &m_fwdPosSingleMatchQualLUT;
    m_lutDict[tftype::emtf_neg+tftype::emtf_neg*5] = &m_fwdNegSingleMatchQualLUT;
    m_lutDict[tftype::omtf_pos+tftype::emtf_pos*5] = &m_foPosMatchQualLUT;
    m_lutDict[tftype::omtf_neg+tftype::emtf_neg*5] = &m_foNegMatchQualLUT;
}

MicroGMTCancelOutUnit::~MicroGMTCancelOutUnit ()
{

}

void
MicroGMTCancelOutUnit::setCancelOutBits(L1TGMTInternalWedges& wedges, tftype trackFinder, cancelmode mode) 
{ 
  std::vector<std::shared_ptr<L1TGMTInternalMuon>> coll1;
  coll1.reserve(3);
  std::vector<std::shared_ptr<L1TGMTInternalMuon>> coll2;
  coll2.reserve(3);
  int maxWedges = 6;
  if (trackFinder == bmtf) {
    maxWedges = 12;
  }
  for (int currentWedge = 1; currentWedge <= maxWedges; ++currentWedge) {
    for (auto mu : wedges.at(currentWedge)) {
      coll1.push_back(mu);
    }
    // handle wrap around: max "wedge" has to be compared to first "wedge"
    int neighbourWedge = (currentWedge % maxWedges) + 1;
    for (auto mu : wedges.at(neighbourWedge)) {
      coll2.push_back(mu);
    }
    if (mode == cancelmode::coordinate) {
      getCoordinateCancelBits(coll1, coll2);
    } else {
      getTrackAddrCancelBits(coll1, coll2);
    }    

    coll1.clear();
    coll2.clear();
  }
}

void
MicroGMTCancelOutUnit::setCancelOutBitsOverlapBarrel(L1TGMTInternalWedges& omtfSectors, L1TGMTInternalWedges& bmtfWedges, cancelmode mode) 
{
  // overlap sector collection
  std::vector<std::shared_ptr<L1TGMTInternalMuon>> coll1;
  coll1.reserve(3);
  // barrel wedge collection with 4 wedges
  std::vector<std::shared_ptr<L1TGMTInternalMuon>> coll2;
  coll2.reserve(12);

  for (int currentSector = 1; currentSector <= 6; ++currentSector) {
    for (auto omtfMuon : omtfSectors.at(currentSector)) {
      coll1.push_back(omtfMuon);
    }
    // BMTF | 2  | 3  | 4  | 5  | 6  | 7  | 8  | 9  | 10 | 11 | 12 | 1  |
    // OMTF |    1    |    2    |    3    |    4    |    5    |    6    |
    // cancel OMTF sector x with corresponding BMTF wedge + the two on either side;
    // e.g. OMTF 1 with BMTF 1, 2, 3, 4, OMTF 2 with BMTF 3, 4, 5, 6 etc.
    for (int i = 0; i < 4; ++i) { 
      int currentWedge = currentSector * 2 - 1 + i;
      // handling the wrap-around: doing a shift by one for the modulo
      // as the wedge numbering starts at 1 instead of 0
      currentWedge = (currentWedge - 1) % 12 + 1;
      for (auto bmtfMuon : bmtfWedges.at(currentWedge)) {
        coll2.push_back(bmtfMuon);
      }
    }
    if (mode == cancelmode::coordinate) {
      getCoordinateCancelBits(coll1, coll2);
    } else {
      getTrackAddrCancelBits(coll1, coll2);
    }
    coll1.clear();
    coll2.clear();
  }
}

void
MicroGMTCancelOutUnit::setCancelOutBitsOverlapEndcap(L1TGMTInternalWedges& omtfSectors, L1TGMTInternalWedges& emtfSectors, cancelmode mode) 
{
  // overlap sector collection
  std::vector<std::shared_ptr<L1TGMTInternalMuon>> coll1;
  coll1.reserve(3);
  // endcap sector collection with 3 sectors
  std::vector<std::shared_ptr<L1TGMTInternalMuon>> coll2;
  coll2.reserve(9);

  for (int curOmtfSector = 1; curOmtfSector <= 6; ++curOmtfSector) {
    for (auto omtfMuon : omtfSectors.at(curOmtfSector)) {
      coll1.push_back(omtfMuon);
    }
    // OMTF |    1    |    2    |    3    |    4    |    5    |    6    |
    // EMTF |    1    |    2    |    3    |    4    |    5    |    6    |
    // cancel OMTF sector x with corresponding EMTF sector + the ones on either side;
    // e.g. OMTF 1 with EMTF 6, 1, 2; OMTF 2 with EMTF 1, 2, 3 etc.
    for (int i = 0; i < 3; ++i) {
      // handling the wrap around: doing shift by 6 (because 1 has to be compared to 6)
      // and the additional shift by one as above because of 1-indexed processor IDs
      int curEmtfSector = ((curOmtfSector + 6) - 1 + i) % 6 + 1;
      for (auto emtfMuon : emtfSectors.at(curEmtfSector)) {
        coll2.push_back(emtfMuon);
      }
    }
    if (mode == cancelmode::coordinate) {
      getCoordinateCancelBits(coll1, coll2);
    } else {
      getTrackAddrCancelBits(coll1, coll2);
    }    coll1.clear();
    coll2.clear();
  }

}

void 
MicroGMTCancelOutUnit::getCoordinateCancelBits(std::vector<std::shared_ptr<L1TGMTInternalMuon>>& coll1, std::vector<std::shared_ptr<L1TGMTInternalMuon>>& coll2)
{
  if (coll1.size() == 0 || coll2.size() == 0) {
    return;
  }
  MicroGMTMatchQualLUT* matchLUT = m_lutDict.at((*coll1.begin())->trackFinderType()+(*coll2.begin())->trackFinderType()*5);
  for (auto mu_w1 = coll1.begin(); mu_w1 != coll1.end(); ++mu_w1) {
    for (auto mu_w2 = coll2.begin(); mu_w2 != coll2.end(); ++mu_w2) {
      // The LUT for cancellation takes reduced width phi and eta, we need the LSBs 
      int dPhiMask = (1 << matchLUT->getDeltaPhiWidth()) - 1;
      int dEtaMask = (1 << matchLUT->getDeltaEtaWidth()) - 1;

      int dPhi = std::abs((*mu_w1)->hwLocalPhi() - (*mu_w2)->hwLocalPhi());
      int dEta = std::abs((*mu_w1)->hwEta() - (*mu_w2)->hwEta()); 
      // check first if the delta is within the LSBs that the LUT takes, otherwise the distance 
      // is greater than what we want to cancel -> 15(int) is max => 15*0.01 = 0.15 (rad)
      if (dEta < dEtaMask && dPhi < dPhiMask) {
        bool match = matchLUT->lookup(dEta & dEtaMask, dPhi & dPhiMask);
        if((*mu_w1)->hwQual() > (*mu_w2)->hwQual() && match) {
          (*mu_w2)->setHwCancelBit(1);
        } else if (match) {
          (*mu_w1)->setHwCancelBit(1);
        }
      }
    }
  }
}

void 
MicroGMTCancelOutUnit::getTrackAddrCancelBits(std::vector<std::shared_ptr<L1TGMTInternalMuon>>& coll1, std::vector<std::shared_ptr<L1TGMTInternalMuon>>& coll2)
{
  // not entirely clear how to do.. just a hook for now
}

} // namespace l1t