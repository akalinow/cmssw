// -*- C++ -*-
//
// Package:    uGMTReadoutTest
// Class:      uGMTReadoutTest
// 
/**\class uGMTReadoutTest uGMTReadoutTest.cc L1Trigger/L1TGlobalMuon/plugins/uGMTReadoutTest.cc

 Description: takes generated muons and fills them in the expected collections for the uGMT

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Joschka Philip Lingemann,40 3-B01,+41227671598,
//         Created:  Thu Oct  3 10:12:30 CEST 2013
// $Id$
//
//


// system include files
#include <memory>
#include <fstream>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DataFormats/L1TMuon/interface/L1TRegionalMuonCandidateFwd.h"
#include "DataFormats/L1TMuon/interface/L1TRegionalMuonCandidate.h"
#include "DataFormats/L1TMuon/interface/L1TGMTInputCaloSumFwd.h"
#include "DataFormats/L1TMuon/interface/L1TGMTInputCaloSum.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/L1Trigger/interface/Muon.h"
//
// class declaration
//
namespace l1t {
class uGMTReadoutTest : public edm::EDProducer {
   public:
      explicit uGMTReadoutTest(const edm::ParameterSet&);
      ~uGMTReadoutTest();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

   private:
      virtual void beginJob() ;
      virtual void produce(edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;
      
      virtual void beginRun(edm::Run&, edm::EventSetup const&);
      virtual void endRun(edm::Run&, edm::EventSetup const&);
      virtual void beginLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&);
      virtual void endLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&);
      
      // ----------member data ---------------------------
      edm::EDGetTokenT <MuonBxCollection> m_gmtInputToken;
};

//
// constants, enums and typedefs
//


//
// static data member definitions
//

//
// constructors and destructor
//
uGMTReadoutTest::uGMTReadoutTest(const edm::ParameterSet& iConfig) 
{
  //register your inputs:
  m_gmtInputToken = consumes <MuonBxCollection> (std::string("microGMTEmulator"));
  //register your products
}


uGMTReadoutTest::~uGMTReadoutTest()
{
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
}


//
// member functions
//





// ------------ method called to produce the data  ------------
void
uGMTReadoutTest::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;

  edm::Handle<MuonBxCollection> gmt;
  if (iEvent.getByToken(m_gmtInputToken, gmt)) {
    std::cout << "n(gmt):" << gmt->size(0) << std::endl;
  }


  edm::Handle<L1TRegionalMuonCandidateCollection> bmutf;
  if (iEvent.getByLabel("uGMTInputProducer", "BarrelTFMuons", bmutf)) {
    std::cout << "n(bmutf):" << bmutf->size() << std::endl;
  }

  edm::Handle<L1TRegionalMuonCandidateCollection> omutf;
  if (iEvent.getByLabel("uGMTInputProducer", "OverlapTFMuons", omutf)) {
    std::cout << "n(omutf):" << omutf->size() << std::endl;
  }

  edm::Handle<L1TRegionalMuonCandidateCollection> emutf;
  if (iEvent.getByLabel("uGMTInputProducer", "ForwardTFMuons", emutf)) {
    std::cout << "n(emutf):" << emutf->size() << std::endl;
  }

  edm::Handle<L1TGMTInputCaloSumCollection> calo;
  if (iEvent.getByLabel("uGMTInputProducer", "TriggerTowerSums", calo)) {
    std::cout << "n(calo):" << calo->size() << std::endl;
  }

 
}


// ------------ method called once each job just before starting event loop  ------------
void 
uGMTReadoutTest::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
uGMTReadoutTest::endJob() {
}

// ------------ method called when starting to processes a run  ------------
void 
uGMTReadoutTest::beginRun(edm::Run&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a run  ------------
void 
uGMTReadoutTest::endRun(edm::Run&, edm::EventSetup const&)
{
}

// ------------ method called when starting to processes a luminosity block  ------------
void 
uGMTReadoutTest::beginLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a luminosity block  ------------
void 
uGMTReadoutTest::endLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&)
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
uGMTReadoutTest::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}
}
//define this as a plug-in
DEFINE_FWK_MODULE(l1t::uGMTReadoutTest);
