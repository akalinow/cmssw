#include <iostream>
#include <strstream>
#include <vector>

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/L1TMuon/interface/L1TRegionalMuonCandidate.h"
#include "DataFormats/L1TMuon/interface/L1TRegionalMuonCandidateFwd.h"

#include "L1Trigger/L1OverlapMuonTrackFinder/plugins/OMTFProducer.h"
#include "L1Trigger/L1OverlapMuonTrackFinder/interface/OMTFProcessor.h"
#include "L1Trigger/L1OverlapMuonTrackFinder/interface/OMTFinputMaker.h"
#include "L1Trigger/L1OverlapMuonTrackFinder/interface/OMTFinput.h"
#include "L1Trigger/L1OverlapMuonTrackFinder/interface/OMTFSorter.h"
#include "L1Trigger/L1OverlapMuonTrackFinder/interface/OMTFConfiguration.h"
#include "L1Trigger/L1OverlapMuonTrackFinder/interface/OMTFConfigMaker.h"
#include "L1Trigger/L1OverlapMuonTrackFinder/interface/XMLConfigWriter.h"

using namespace L1TMuon;

OMTFProducer::OMTFProducer(const edm::ParameterSet& cfg):
  theConfig(cfg),
  trigPrimSrc(cfg.getParameter<edm::InputTag>("TriggerPrimitiveSrc")){

  produces<l1t::L1TRegionalMuonCandidateCollection >("OMTF");

  inputToken = consumes<TriggerPrimitiveCollection>(trigPrimSrc);
  
  if(!theConfig.exists("omtf")){
    edm::LogError("OMTFProducer")<<"omtf configuration not found in cfg.py";
  }
  
  myInputMaker = new OMTFinputMaker();
  mySorter = new OMTFSorter();
  myWriter = 0;
  
  dumpResultToXML = theConfig.getParameter<bool>("dumpResultToXML");
  dumpGPToXML = theConfig.getParameter<bool>("dumpGPToXML");
  theConfig.getParameter<std::string>("XMLDumpFileName");
  
  if(dumpResultToXML || dumpGPToXML){
    myWriter = new XMLConfigWriter();
    std::string fName = "OMTF_Events";
    myWriter->initialiseXMLDocument(fName);
  }

  myOMTFConfig = 0;
}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
OMTFProducer::~OMTFProducer(){

  delete myOMTFConfig;
  delete myOMTFConfigMaker;
  delete myOMTF;

  delete myInputMaker;
  delete mySorter;

  if (myWriter) delete myWriter;

}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
void OMTFProducer::beginJob(){

  if(theConfig.exists("omtf")){
    myOMTFConfig = new OMTFConfiguration(theConfig.getParameter<edm::ParameterSet>("omtf"));
    myOMTFConfigMaker = new OMTFConfigMaker(theConfig.getParameter<edm::ParameterSet>("omtf"));
    myOMTF = new OMTFProcessor(theConfig.getParameter<edm::ParameterSet>("omtf"));
  }
}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////  
void OMTFProducer::endJob(){

  if(dumpResultToXML && !dumpGPToXML){
    std::string fName = theConfig.getParameter<std::string>("XMLDumpFileName");
    myWriter->finaliseXMLDocument(fName);
  }

  if(dumpGPToXML && !dumpResultToXML){
    std::string fName = "OMTF";
    myWriter->initialiseXMLDocument(fName);
    const std::map<Key,GoldenPattern*> & myGPmap = myOMTF->getPatterns();
    for(auto itGP: myGPmap){
      //std::cout<<*itGP.second<<std::endl;     
      myWriter->writeGPData(*itGP.second);
    }
    fName = "GPs.xml";
    myWriter->finaliseXMLDocument(fName);
    ///Write merged GPs.//////////////////////////////////
    ///2x merging
    int charge = -1;
    int iPtMin = 4;
    Key aKey(1, iPtMin, charge);    

    fName = "OMTF";
    myWriter->initialiseXMLDocument(fName);
    while(myGPmap.find(aKey)!=myGPmap.end()){
      
      GoldenPattern *aGP1 = myGPmap.find(aKey)->second;
      GoldenPattern *aGP2 = aGP1;

      ++aKey.thePtCode;
      if(aKey.thePtCode<=31 && myGPmap.find(aKey)!=myGPmap.end()) aGP2 =  myGPmap.find(aKey)->second;      

      myWriter->writeGPData(*aGP1,*aGP2);

      aKey.theCharge = 1;
      if(aKey.thePtCode<=31 && myGPmap.find(aKey)!=myGPmap.end()) aGP2 = myGPmap.find(aKey)->second;
      --aKey.thePtCode;
      if(myGPmap.find(aKey)!=myGPmap.end()) aGP1 = myGPmap.find(aKey)->second;
      myWriter->writeGPData(*aGP1,*aGP2);

      ++aKey.thePtCode;
      ++aKey.thePtCode;
      aKey.theCharge = -1;
      
    }   
    fName = "GPs_2x.xml";
    myWriter->finaliseXMLDocument(fName);  
    //////////////////////////////////////////////
    ///4x merging
    fName = "OMTF";
    myWriter->initialiseXMLDocument(fName);

    GoldenPattern *dummy = new GoldenPattern(Key(0,0,0));
    dummy->reset();

    aKey = Key(1, iPtMin, charge);    
    while(myGPmap.find(aKey)!=myGPmap.end()){

    GoldenPattern *aGP1 = myGPmap.find(aKey)->second;
    GoldenPattern *aGP2 = aGP1;
    GoldenPattern *aGP3 = aGP1;
    GoldenPattern *aGP4 = aGP1;

    ++aKey.thePtCode;
    if(aKey.thePtCode<=31 && myGPmap.find(aKey)!=myGPmap.end()) aGP2 =  myGPmap.find(aKey)->second;
    
    if(aKey.thePtCode>19){
      ++aKey.thePtCode;
      if(aKey.thePtCode<=31 && myGPmap.find(aKey)!=myGPmap.end()) aGP3 =  myGPmap.find(aKey)->second;
      
      ++aKey.thePtCode;
      if(aKey.thePtCode<=31 && myGPmap.find(aKey)!=myGPmap.end()) aGP4 =  myGPmap.find(aKey)->second;
    }
    else{
      aGP3 = dummy;
      aGP4 = dummy;
    }
    ++aKey.thePtCode;


    myWriter->writeGPData(*aGP1,*aGP2, *aGP3, *aGP4);
    }   
    ///
    aKey = Key(1, iPtMin,1);    
    while(myGPmap.find(aKey)!=myGPmap.end()){

    GoldenPattern *aGP1 = myGPmap.find(aKey)->second;
    GoldenPattern *aGP2 = aGP1;
    GoldenPattern *aGP3 = aGP1;
    GoldenPattern *aGP4 = aGP1;

    ++aKey.thePtCode;
    if(aKey.thePtCode<=31 && myGPmap.find(aKey)!=myGPmap.end()) aGP2 =  myGPmap.find(aKey)->second;
    
    if(aKey.thePtCode>19){
      ++aKey.thePtCode;
      if(aKey.thePtCode<=31 && myGPmap.find(aKey)!=myGPmap.end()) aGP3 =  myGPmap.find(aKey)->second;
      
      ++aKey.thePtCode;
      if(aKey.thePtCode<=31 && myGPmap.find(aKey)!=myGPmap.end()) aGP4 =  myGPmap.find(aKey)->second;
    }
    else{
      aGP3 = dummy;
      aGP4 = dummy;
    }
    ++aKey.thePtCode;

    myWriter->writeGPData(*aGP1,*aGP2, *aGP3, *aGP4);
    } 
    fName = "GPs_4x.xml";
    myWriter->finaliseXMLDocument(fName); 
  }
}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////  
void OMTFProducer::produce(edm::Event& iEvent, const edm::EventSetup& evSetup){

  std::ostringstream myStr;

  myInputMaker->initialize(evSetup);

  edm::Handle<TriggerPrimitiveCollection> trigPrimitives;
  iEvent.getByToken(inputToken, trigPrimitives);

  ///Filter digis by dropping digis from selected (by cfg.py) subsystems
  const L1TMuon::TriggerPrimitiveCollection filteredDigis = filterDigis(*trigPrimitives);

  std::auto_ptr<l1t::L1TRegionalMuonCandidateCollection > myCands(new l1t::L1TRegionalMuonCandidateCollection);

  if(dumpResultToXML) aTopElement = myWriter->writeEventHeader(iEvent.id().event());

  ///Loop over all processors, each covering 60 deg in phi
  for(unsigned int iProcessor=0;iProcessor<6;++iProcessor){
    
    myStr<<" iProcessor: "<<iProcessor;
    
    const OMTFinput *myInput = myInputMaker->buildInputForProcessor(filteredDigis,iProcessor);
       
    ///Input data with phi ranges shifted for each processor, so it fits 11 bits range
    OMTFinput myShiftedInput =  myOMTF->shiftInput(iProcessor,*myInput);	

    ///Results for each GP in each logic region of given processor
    const std::vector<OMTFProcessor::resultsMap> & myResults = myOMTF->processInput(iProcessor,myShiftedInput);

    //Retreive all candidates returned by sorter: upto 3 non empty ones with different phi or charge
    l1t::L1TRegionalMuonCandidateCollection myOTFCandidates;
    mySorter->sortProcessor(myResults,myOTFCandidates);

    ////Switch from internal processor n bit scale to global one
    int procOffset = OMTFConfiguration::globalPhiStart(iProcessor);
    int lowScaleEnd = pow(2,OMTFConfiguration::nPhiBits-1);
    if(procOffset<0) procOffset+=(int)OMTFConfiguration::nPhiBins;

    //dumpResultToXML = false;

    for(unsigned int iCand=0; iCand<myOTFCandidates.size(); ++iCand){
      // shift phi from processor to global coordinates
      int phiValue = (myOTFCandidates[iCand].hwPhi()+procOffset+lowScaleEnd);
      if(phiValue>=(int)OMTFConfiguration::nPhiBins) phiValue-=OMTFConfiguration::nPhiBins;
      phiValue/=10; //uGMT has 10x coarser scale than OMTF
      myOTFCandidates[iCand].setHwPhi(phiValue);
      // store candidate 
      if(myOTFCandidates[iCand].hwPt()){
	myCands->push_back(myOTFCandidates[iCand]);       
	myStr<<" Candidate pt code: "<<myOTFCandidates[iCand].hwPt();
      }
    }
    ///Write to XML
    if(dumpResultToXML){
      xercesc::DOMElement * aProcElement = myWriter->writeEventData(aTopElement,iProcessor,myShiftedInput);
      for(unsigned int iRefHit=0;iRefHit<OMTFConfiguration::nTestRefHits;++iRefHit){
	///Dump only regions, where a candidate was found
	InternalObj myCand = mySorter->sortRefHitResults(myResults[iRefHit],0);//charge=0 means ignore charge
	if(myCand.pt){
	  myWriter->writeCandidateData(aProcElement,iRefHit,myCand);
	  /*
	  for(auto & itKey: myResults[iRefHit]) myWriter->writeResultsData(aProcElement, 
									   iRefHit,
									   itKey.first,itKey.second);    
	  */
	}
      }
    }
  }

  //dumpResultToXML = true;

  myStr<<" Number of candidates: "<<myCands->size();
  edm::LogInfo("OMTFOMTFProducer")<<myStr.str();

  iEvent.put(myCands, "OMTF");
}
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////  
const L1TMuon::TriggerPrimitiveCollection OMTFProducer::filterDigis(const L1TMuon::TriggerPrimitiveCollection & vDigi){

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
