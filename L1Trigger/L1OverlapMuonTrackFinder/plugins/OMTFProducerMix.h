
#ifndef OMTFProducerMix_H
#define OMTFProducerMix_H

#include "xercesc/util/XercesDefs.hpp"

#include "DataFormats/L1TMuon/interface/L1TMuonTriggerPrimitive.h"
#include "DataFormats/L1TMuon/interface/L1TMuonTriggerPrimitiveFwd.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/EDProducer.h"

#include "TRandom3.h"

class OMTFProcessor;
class OMTFConfiguration;
class OMTFConfigMaker;
class OMTFinputMaker;
class OMTFSorter;
class OMTFinput;

class XMLConfigWriter;
class XMLConfigReader;

namespace XERCES_CPP_NAMESPACE{
  class DOMElement;
  class DOMDocument;
  class DOMImplementation;
}


class OMTFProducerMix : public edm::EDProducer {
 public:
  OMTFProducerMix(const edm::ParameterSet&);
  
  ~OMTFProducerMix();

  virtual void beginJob();

  virtual void endJob();
  
  virtual void produce(edm::Event&, const edm::EventSetup&);  

 private:

  edm::ParameterSet theConfig;
  edm::InputTag trigPrimSrc;
  edm::EDGetTokenT<L1TMuon::TriggerPrimitiveCollection> inputToken;

  const L1TMuon::TriggerPrimitiveCollection filterDigis(const L1TMuon::TriggerPrimitiveCollection & vDigi);

  ///OMTF objects
  OMTFConfiguration *myOMTFConfig;
  OMTFinputMaker *myInputMaker;
  OMTFSorter *mySorter;
  OMTFProcessor *myOMTF;
  OMTFinput *myInputXML;
  ///
  xercesc::DOMElement *aTopElement;
  OMTFConfigMaker *myOMTFConfigMaker;
  XMLConfigWriter *myWriter; 
  XMLConfigReader *myReader; 
  ///
  unsigned int eventsToMix;
  bool dumpResultToXML;
  TRandom3 aRndm;


};

#endif
