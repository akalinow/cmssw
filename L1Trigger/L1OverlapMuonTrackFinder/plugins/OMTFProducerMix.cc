#include <iostream>
#include <strstream>
#include <vector>

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/L1TMuon/interface/L1TRegionalMuonCandidate.h"
#include "DataFormats/L1TMuon/interface/L1TRegionalMuonCandidateFwd.h"

#include "L1Trigger/L1OverlapMuonTrackFinder/plugins/OMTFProducerMix.h"
#include "L1Trigger/L1OverlapMuonTrackFinder/interface/OMTFProcessor.h"
#include "L1Trigger/L1OverlapMuonTrackFinder/interface/OMTFinputMaker.h"
#include "L1Trigger/L1OverlapMuonTrackFinder/interface/OMTFinput.h"
#include "L1Trigger/L1OverlapMuonTrackFinder/interface/OMTFSorter.h"
#include "L1Trigger/L1OverlapMuonTrackFinder/interface/OMTFConfiguration.h"
#include "L1Trigger/L1OverlapMuonTrackFinder/interface/OMTFConfigMaker.h"
#include "L1Trigger/L1OverlapMuonTrackFinder/interface/XMLConfigWriter.h"
#include "L1Trigger/L1OverlapMuonTrackFinder/interface/XMLConfigReader.h"

using namespace L1TMuon;

OMTFProducerMix::OMTFProducerMix(const edm::ParameterSet& cfg):
  theConfig(cfg),
  trigPrimSrc(cfg.getParameter<edm::InputTag>("TriggerPrimitiveSrc")){

  produces<l1t::L1TRegionalMuonCandidateCollection >("OMTF");

  inputToken = consumes<TriggerPrimitiveCollection>(trigPrimSrc);
  dumpResultToXML = theConfig.getParameter<bool>("dumpResultToXML");
  
  if(!theConfig.exists("omtf")){
    edm::LogError("OMTFProducerMix")<<"omtf configuration not found in cfg.py";
  }
  
  myInputMaker = new OMTFinputMaker();
  mySorter = new OMTFSorter();
  myWriter = 0;
  myReader = 0;
  
  myInputXML = new OMTFinput();
  myReader = new XMLConfigReader();
  if(dumpResultToXML){
    myWriter = new XMLConfigWriter();
    std::string fName = "OMTF_Events";
    myWriter->initialiseXMLDocument(fName);
  }
  
  std::vector<std::string> fileNames = theConfig.getParameter<std::vector<std::string> >("eventsXMLFiles");
  for(auto it: fileNames) myReader->setEventsFile(it);
  eventsToMix = theConfig.getParameter<unsigned int>("eventsToMix");

  myOMTFConfig = 0;
}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
OMTFProducerMix::~OMTFProducerMix(){

  delete myOMTFConfig;
  delete myOMTFConfigMaker;
  delete myOMTF;

  delete myInputMaker;
  delete mySorter;

  if(myWriter) delete myWriter;
  delete myReader;
  delete myInputXML;
}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
void OMTFProducerMix::beginJob(){

  if(theConfig.exists("omtf")){
    myOMTFConfig = new OMTFConfiguration(theConfig.getParameter<edm::ParameterSet>("omtf"));
    myOMTFConfigMaker = new OMTFConfigMaker(theConfig.getParameter<edm::ParameterSet>("omtf"));
    myOMTF = new OMTFProcessor(theConfig.getParameter<edm::ParameterSet>("omtf"));
  }
}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////  
void OMTFProducerMix::endJob(){ 

  if(dumpResultToXML){
    std::string fName = "MixedEvents.xml";
    myWriter->finaliseXMLDocument(fName);
  }
}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////  
void OMTFProducerMix::produce(edm::Event& iEvent, const edm::EventSetup& evSetup){

  std::ostringstream myStr;

  myInputMaker->initialize(evSetup);

  edm::Handle<TriggerPrimitiveCollection> trigPrimitives;
  iEvent.getByToken(inputToken, trigPrimitives);

  ///Filter digis by dropping digis from selected (by cfg.py) subsystems
  const L1TMuon::TriggerPrimitiveCollection filteredDigis = filterDigis(*trigPrimitives);

  std::auto_ptr<l1t::L1TRegionalMuonCandidateCollection > myCands(new l1t::L1TRegionalMuonCandidateCollection);

  ///Loop over events to be mixed with current EDM event
  for(unsigned int iEventMix=0;iEventMix<=2*eventsToMix;++iEventMix){

    if(dumpResultToXML) aTopElement = myWriter->writeEventHeader(iEvent.id().event(), iEventMix);

    ///Loop over all processors, each covering 60 deg in phi
    for(unsigned int iProcessor=0;iProcessor<6;++iProcessor){
    
      myStr<<" iProcessor: "<<iProcessor;
    
      const OMTFinput *myInput = myInputMaker->buildInputForProcessor(filteredDigis,iProcessor);
       
      ///Input data with phi ranges shifted for each processor, so it fits 11 bits range
      OMTFinput myShiftedInput =  myOMTF->shiftInput(iProcessor,*myInput);	
      
      ///Every second BX contains the mixed event
      if(iEventMix%2==1 && iEventMix>0) myShiftedInput.clear();
      ///First BX contains the original event
      if(iEventMix>0){
	myInputXML->clear();
	myInputXML->readData(myReader,int(iEventMix-0.5)/2, iProcessor);
	myShiftedInput.mergeData(myInputXML);
      }
      ///Results for each GP in each logic region of given processor
      const std::vector<OMTFProcessor::resultsMap> & myResults = myOMTF->processInput(iProcessor,myShiftedInput);
      
      //Retreive all candidates returned by sorter: upto 3 non empty ones with different phi or charge
      l1t::L1TRegionalMuonCandidateCollection  myOTFCandidates;
      mySorter->sortProcessor(myResults,myOTFCandidates);
      
      ////Switch from internal processor n bit scale to global one
      int procOffset = OMTFConfiguration::globalPhiStart(iProcessor);
      int lowScaleEnd = pow(2,OMTFConfiguration::nPhiBits-1);
      
      if(procOffset<0) procOffset+=OMTFConfiguration::nPhiBins;
      
      for(unsigned int iCand=0; iCand<myOTFCandidates.size(); ++iCand){
	// shift phi from processor to global coordinates
	int phiValue = (myOTFCandidates[iCand].hwPhi()+OMTFConfiguration::globalPhiStart(iProcessor)+lowScaleEnd);
	myOTFCandidates[iCand].setHwPhi(phiValue);
	myOTFCandidates[iCand].setHwSignValid(iEventMix);
	// store candidate 
	if(myOTFCandidates[iCand].hwPt()) myCands->push_back(myOTFCandidates[iCand]);       
      }

      ///Write to XML
      if(dumpResultToXML){
	xercesc::DOMElement * aProcElement = myWriter->writeEventData(aTopElement,iProcessor,myShiftedInput);
	for(unsigned int iRefHit=0;iRefHit<OMTFConfiguration::nTestRefHits;++iRefHit){
	  ///Dump only regions, where a candidate was found
	  InternalObj myCand = mySorter->sortRefHitResults(myResults[iRefHit],0);//charge=0 means ignore charge
	  if(myCand.pt){
	    myWriter->writeCandidateData(aProcElement,iRefHit,myCand);
	  }
	}
      }
      edm::LogInfo("OMTFOMTFProducerMix")<<myStr.str();
      myStr.clear();
    }
  }
  
  myStr<<" Number of candidates: "<<myCands->size();
  edm::LogInfo("OMTFOMTFProducerMix")<<myStr.str();

  iEvent.put(myCands, "OMTF");  

}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////  
const L1TMuon::TriggerPrimitiveCollection OMTFProducerMix::filterDigis(const L1TMuon::TriggerPrimitiveCollection & vDigi){

  if(!theConfig.getParameter<bool>("dropRPCPrimitives") &&
     !theConfig.getParameter<bool>("dropDTPrimitives") &&
     !theConfig.getParameter<bool>("dropCSCPrimitives")) return vDigi;
  
  L1TMuon::TriggerPrimitiveCollection filteredDigis;
  for(auto it:vDigi){
    switch (it.subsystem()) {
    case L1TMuon::TriggerPrimitive::kRPC: {
      if(!theConfig.getParameter<bool>("dropRPCPrimitives")) filteredDigis.push_back(it);
      break;
    }
    case L1TMuon::TriggerPrimitive::kDT: {
      if(!theConfig.getParameter<bool>("dropDTPrimitives")) filteredDigis.push_back(it);
      break;
    }
    case L1TMuon::TriggerPrimitive::kCSC: {
      if(!theConfig.getParameter<bool>("dropCSCPrimitives")) filteredDigis.push_back(it);
      break;
    }
    case L1TMuon::TriggerPrimitive::kNSubsystems: {break;} 
    }
  }
  return filteredDigis;
}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////  
