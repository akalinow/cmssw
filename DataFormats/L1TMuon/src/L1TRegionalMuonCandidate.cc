#include "DataFormats/L1TMuon/interface/L1TRegionalMuonCandidate.h"
#include <iostream> 

namespace l1t {

void
L1TRegionalMuonCandidate::setTFIdentifiers(int processor, tftype trackFinder) {
  m_trackFinder = trackFinder;
  m_processor = processor;

  switch (m_trackFinder) {
    case tftype::bmtf:
      m_link = m_processor + 35;  // range 36...47
    case tftype::omtf_pos:
      m_link = m_processor + 47;  // range 48...53
    case tftype::omtf_neg:
      m_link = m_processor + 53;  // range 54...59
    case tftype::emtf_pos:
      m_link = m_processor + 59;  // range 60...65
    case tftype::emtf_neg:
      m_link = m_processor + 65;  // range 66...71
  }
}

void 
L1TRegionalMuonCandidate::setLink(int link) {
  std::cout << "Please move to setTFIdentifiers" << std::endl;
  m_link = link; 
}
} // namespace l1t