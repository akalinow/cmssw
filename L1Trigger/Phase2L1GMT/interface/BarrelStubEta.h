#ifndef L1Trigger_Phase2L1GMT_BarrelStubEta_h
#define L1Trigger_Phase2L1GMT_BarrelStubEta_h

#include <cmath>
#include "DataFormats/L1TMuonPhase2/interface/Constants.h"

namespace Phase2L1GMT {

  //Since CMSSW_20 barrel stubs carry z and k=dz/dR of the DT segment in the eta1 and eta2 fields
  //(L1TPhase2GMTBarrelStubProcessor::buildStubwithZandkSlope). Convert z back to the stub eta
  //(units of ETADIVIDER*LSBeta) pointing from the origin to the segment, as the pre-CMSSW_20 eta LUTs did.
  //In firmware this is a LUT addressed by (station, z).
  inline ap_int<BITSSTUBETA> barrelStubEta(int z, unsigned int tfLayer) {
    static constexpr float RadiusStationPhys[] = {445., 526., 635., 730.};
    static constexpr float zLSB = 1500. / (1 << 16);
    static const float stubEtaLSB = (1 << (BITSETA - BITSSTUBETA)) * 2 * M_PI / (1 << BITSETA);
    float eta = std::asinh(z * zLSB / RadiusStationPhys[tfLayer]);
    return ap_int<BITSSTUBETA>(std::lround(eta / stubEtaLSB));
  }

}  // namespace Phase2L1GMT

#endif
