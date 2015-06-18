#include "L1Trigger/L1EndcapMuonTrackFinder/interface/PtAssignmentUnit.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

using namespace L1TMuon;

PtAssignmentUnit::PtAssignmentUnit(const edm::ParameterSet& ps):
  _name(ps.getParameter<std::string>("name")) {
}
