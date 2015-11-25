#ifndef ANGLECONVERTER_H
#define ANGLECONVERTER_H

#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "DataFormats/L1TMuon/interface/RegionalMuonCandFwd.h"

#include <memory>

namespace edm {  
  class EventSetup;
}

class RPCGeometry;
class CSCGeometry;
class CSCLayer;
class DTGeometry;

class L1MuDTChambPhDigi;
class L1MuDTChambThContainer;
class CSCCorrelatedLCTDigi;
class RPCDigi;
class CSCDetId;
class RPCDetId;

  class AngleConverter {
  public:
    AngleConverter();
    ~AngleConverter();

    ///Update the Geometry with current Event Setup
    void checkAndUpdateGeometry(const edm::EventSetup&);

    /// get phi of DT,CSC and RPC azimutal angle digi in processor scale, used by OMTF algorithm.
    /// in case of wrong phi returns OMTFConfiguration::nPhiBins
    int getProcessorPhi(unsigned int iProcessor, l1t::tftype part, const L1MuDTChambPhDigi &digi) const;
    int getProcessorPhi(unsigned int iProcessor, l1t::tftype part, const CSCDetId & csc, const CSCCorrelatedLCTDigi &digi) const;
    int getProcessorPhi(unsigned int iProcessor, l1t::tftype part, const RPCDetId & rollId, const RPCDigi &digi) const;


    ///Convert local phi coordinate to global digital OMTF scale.
    int getGlobalPhi(unsigned int rawid, const L1MuDTChambPhDigi &aDigi);

    ///Convert local phi coordinate to global digital OMTF scale.
    int getGlobalPhi(unsigned int rawid, const CSCCorrelatedLCTDigi &aDigi);

    ///Convert local phi coordinate to global digital OMTF scale.
    ///To maintain backward comtability return float value fo global
    ///phi. Later whewn LUT will be used will return int as other methods.
    float getGlobalPhi(unsigned int rawid, const RPCDigi &aDigi);

    ///Convert local eta coordinate to global digital microGMT scale.
    int getGlobalEta(unsigned int rawid, const L1MuDTChambPhDigi &aDigi,
		     const L1MuDTChambThContainer *dtThDigis);
    
    ///Convert local eta coordinate to global digital microGMT scale.
    int getGlobalEta(unsigned int rawid, const CSCCorrelatedLCTDigi &aDigi);
    
    ///Convert local eta coordinate to global digital microGMT scale.
    int getGlobalEta(unsigned int rawid, const RPCDigi &aDigi);

  private:

    ///Check orientation of strips in given CSC chamber
    bool isCSCCounterClockwise(const std::unique_ptr<const CSCLayer>& layer) const;


    ///Find BTI group
    const int findBTIgroup(const L1MuDTChambPhDigi &aDigi,
			   const L1MuDTChambThContainer *dtThDigis);
    
    // pointers to the current geometry records
    unsigned long long _geom_cache_id;
    edm::ESHandle<RPCGeometry> _georpc;    
    edm::ESHandle<CSCGeometry> _geocsc;    
    edm::ESHandle<DTGeometry>  _geodt;    
    
   
  };

#endif