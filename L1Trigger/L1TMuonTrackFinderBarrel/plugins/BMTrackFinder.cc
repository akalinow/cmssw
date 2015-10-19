//-------------------------------------------------
//
//   Class: BMTrackFinder
//
//   L1 BM Track Finder EDProducer
//
//
//
//   Author :
//   J. Troconiz              UAM Madrid
//   Modified :
//   G. Flouris               U Ioannina
//--------------------------------------------------

#include "BMTrackFinder.h"

#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/Event.h"

#include "DataFormats/L1DTTrackFinder/interface/L1MuDTChambPhContainer.h"
#include "DataFormats/L1DTTrackFinder/interface/L1MuDTChambThContainer.h"
#include "DataFormats/L1TMuon/interface/BMTrackContainer.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/L1CSCTrackFinder/interface/TrackStub.h"
//#include "DataFormats/L1CSCTrackFinder/interface/CSCTriggerContainer.h"


#include "../src/L1MuBMTFConfig.h"
#include "../interface/L1MuBMTFSetup.h"
#include "../interface/L1MuBMTrackFinder.h"

#include <iostream>
#include <iomanip>

using namespace std;

BMTrackFinder::BMTrackFinder(const edm::ParameterSet & pset) {

  produces<BMTrackContainer>("BMTF");
  produces<l1t::RegionalMuonCandBxCollection>("BM");


  setup1 = new L1MuBMTFSetup(pset,consumesCollector());
  usesResource("BMTrackFinder");
  consumes<L1MuDTChambPhContainer>(pset.getParameter<edm::InputTag>("DTDigi_Source"));
  consumes<L1MuDTChambThContainer>(pset.getParameter<edm::InputTag>("DTDigi_Source"));
//  consumes<CSCTriggerContainer<csctf::TrackStub>>(pset.getParameter<edm::InputTag>("CSCStub_Source"));

}

BMTrackFinder::~BMTrackFinder() {

  delete setup1;

}

void BMTrackFinder::produce(edm::Event& e, const edm::EventSetup& c) {

  if ( L1MuBMTFConfig::Debug(1) ) cout << endl;
  if ( L1MuBMTFConfig::Debug(1) ) cout << "**** L1MuonBMTFTrigger processing event  ****" << endl;

  L1MuBMTrackFinder* dtbx = setup1->TrackFinder();
  dtbx->clear();

  dtbx->run(e,c);

  int ndt = dtbx->numberOfTracks();
  if ( L1MuBMTFConfig::Debug(1) ) cout << "Number of muons found by the L1 BBMX TRIGGER : "
                                       << ndt << endl;

  auto_ptr<BMTrackContainer> tra_product(new BMTrackContainer);
  std::auto_ptr<l1t::RegionalMuonCandBxCollection> vec_product(new l1t::RegionalMuonCandBxCollection);

  vector<BMTrackCand>  dtTracks = dtbx->getcache0();
  tra_product->setContainer(dtTracks);
  l1t::RegionalMuonCandBxCollection& BMTracks = dtbx->getcache();

  *vec_product = BMTracks;

  e.put(tra_product,"BMTF");
  e.put(vec_product,"BM");

}
