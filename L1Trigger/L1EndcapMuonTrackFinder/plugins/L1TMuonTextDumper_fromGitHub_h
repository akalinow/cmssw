#ifndef _L1ITMu_L1TMuonTextDumper_h_
#define _L1ITMu_L1TMuonTextDumper_h_
//asd
#include <memory>
#include <map>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/EDProducer.h"

#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "L1Trigger/L1TMuon/interface/SubsystemCollectorFactory.h"

#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include <TMath.h>
#include <TCanvas.h>
#include <TLorentzVector.h>

#include <TStyle.h>
#include <TLegend.h>
#include <TF1.h>
#include <TH2.h>
#include <TH1F.h>
#include <TFile.h>
#include "L1Trigger/L1TMuon/interface/GeometryTranslator.h"

#include "DataFormats/L1TMuon/interface/L1TMuonTriggerPrimitive.h"
#include "DataFormats/L1TMuon/interface/L1TMuonTriggerPrimitiveFwd.h"

#include "DataFormats/L1TMuon/interface/L1TMuonInternalTrack.h"
#include "DataFormats/L1TMuon/interface/L1TMuonInternalTrackFwd.h"


#include "PhiMemoryImage.h"
#include "EmulatorClasses.h"
#include "L1Trigger/CSCTrackFinder/interface/CSCTFPtLUT.h"
#include "L1Trigger/CSCTrackFinder/interface/CSCSectorReceiverLUT.h"

typedef edm::ParameterSet PSet;


//class L1TMuonTextDumper : public edm::EDAnalyzer {

class L1TMuonTextDumper : public edm::EDProducer {
public:
  L1TMuonTextDumper(const PSet&);
  ~L1TMuonTextDumper() {}
	
 //void analyze(const edm::Event&, const edm::EventSetup&); 
 void produce(edm::Event&, const edm::EventSetup&); 
  void beginJob();
  void endJob();
  
  FILE *write, *dphi, *tptest;

  ///////////////////////////////////////
  //// For Emulator with timing /////////
  /////  we need all of these ///////////
  ///////////////////////////////////////
 // MatchingOutput Mout;
 // ZonesOutput Zout;
 // ExtenderOutput Eout;
 // PatternOutput Pout;
 // SortingOutput Sout;
 // std::vector<ConvertedHit> ConvHits;
 // std::vector<std::vector<DeltaOutput>> Dout;
  ///////////////////////////////////////
  ///////////////////////////////////////
  ///////////////////////////////////////
  ///////////////////////////////////////
  
  edm::ParameterSet LUTparam;
  CSCSectorReceiverLUT* srLUTs_[5][2];
  const float ptscale[33] = { 
  	-1.,   0.0,   1.5,   2.0,   2.5,   3.0,   3.5,   4.0,
    4.5,   5.0,   6.0,   7.0,   8.0,  10.0,  12.0,  14.0,  
    16.0,  18.0,  20.0,  25.0,  30.0,  35.0,  40.0,  45.0, 
    50.0,  60.0,  70.0,  80.0,  90.0, 100.0, 120.0, 140.0, 1.E6 };
    
    int gpire[24]; 
    int fpire[24];
    
   
   TH1F* striph, *eff, *eff2, *trigprimsize, *trigprimsize2, *st_cont, *st_cont2, *sector1, *sector2, *secdiff;
   TH1F* MissVsEta, *MissVsPhi, *MissVsPt, *ME42test1, *ME42test2, *ME1gangnedtest, *ME11gangnedtest;
   
   TH1F* st12errors, *detectorineff;
  

private:
  bool _dogen;
  edm::InputTag _geninput;
  std::vector<edm::InputTag> _tpinputs, _convTrkInputs;
  edm::Service<TFileService> histofile;
  
};




#endif
