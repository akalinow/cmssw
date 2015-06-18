// -*- C++ -*-
//
// Package:    MicroGMTEmulator
// Class:      MicroGMTEmulator
// 
/**\class MicroGMTEmulator MicroGMTEmulator.cc L1Trigger/L1TMuon/src/MicroGMTEmulator.cc

 Description: Takes txt-file input and produces barrel- / overlap- / forward TF muons 

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Joschka Philip Lingemann,40 3-B01,+41227671598,
//         Created:  Thu Oct  3 16:31:34 CEST 2013
// $Id$
//
//


// system include files
#include <memory>
#include <fstream>
#include <sstream>


// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "L1Trigger/L1TMuon/interface/MicroGMTConfiguration.h"
#include "L1Trigger/L1TMuon/interface/MicroGMTRankPtQualLUT.h"
#include "L1Trigger/L1TMuon/interface/MicroGMTIsolationUnit.h"
#include "L1Trigger/L1TMuon/interface/MicroGMTCancelOutUnit.h"

#include "DataFormats/Math/interface/LorentzVector.h"
#include "DataFormats/L1Trigger/interface/Muon.h"
#include "DataFormats/L1TMuon/interface/L1TRegionalMuonCandidate.h"
#include "DataFormats/L1TMuon/interface/L1TGMTInternalMuon.h"

#include "TMath.h"
//
// class declaration
//
namespace l1t {
  class MicroGMTEmulator : public edm::EDProducer {
     public:
        explicit MicroGMTEmulator(const edm::ParameterSet&);
        ~MicroGMTEmulator();

        static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

     private:
        virtual void beginJob() ;
        virtual void produce(edm::Event&, const edm::EventSetup&);
        virtual void endJob() ;
        
        virtual void beginRun(edm::Run&, edm::EventSetup const&);
        virtual void endRun(edm::Run&, edm::EventSetup const&);
        virtual void beginLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&);
        virtual void endLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&);

        static bool compareMuons(const std::shared_ptr<MicroGMTConfiguration::InterMuon>& mu1, 
                                const std::shared_ptr<MicroGMTConfiguration::InterMuon>& mu2);

        void sortMuons(MicroGMTConfiguration::InterMuonList&, unsigned) const;

        void calculateRank(MicroGMTConfiguration::InterMuonList& muons) const;

        void splitAndConvertMuons(MicroGMTConfiguration::InputCollection const& in, 
                                  MicroGMTConfiguration::InterMuonList& out_pos, 
                                  MicroGMTConfiguration::InterMuonList& out_neg,
                                  L1TGMTInternalWedges& wedges_pos, 
                                  L1TGMTInternalWedges& wedges_neg) const;

        void convertMuons(MicroGMTConfiguration::InputCollection const& in, 
                          MicroGMTConfiguration::InterMuonList& out,
                          L1TGMTInternalWedges& wedges) const;

        void addMuonsToCollections(MicroGMTConfiguration::InterMuonList& coll, 
                                   MicroGMTConfiguration::InterMuonList& interout, 
                                   std::auto_ptr<MuonBxCollection>& out) const;
        
        // ----------member data ---------------------------
        edm::InputTag m_barrelTfInputTag;
        edm::InputTag m_overlapTfInputTag;
        edm::InputTag m_forwardTfInputTag;
        edm::InputTag m_trigTowerTag;
        MicroGMTRankPtQualLUT m_rankPtQualityLUT;
        MicroGMTIsolationUnit m_isolationUnit;
        MicroGMTCancelOutUnit m_cancelOutUnit;
        std::ofstream m_debugOut;

  };
}
//
// constants, enums and typedefs
//


//
// static data member definitions
//

//
// constructors and destructor
//
l1t::MicroGMTEmulator::MicroGMTEmulator(const edm::ParameterSet& iConfig) : m_rankPtQualityLUT(iConfig), m_isolationUnit(iConfig), m_cancelOutUnit(iConfig), m_debugOut("test/debug/iso_debug.dat")
{
  // edm::InputTag barrelTfInputTag = iConfig.getParameter<edm::InputTag>("barrelTFInput");
  // edm::InputTag overlapTfInputTag = iConfig.getParameter<edm::InputTag>("overlapTFInput");
  // edm::InputTag forwardTfInputTag = iConfig.getParameter<edm::InputTag>("forwardTFInput");

  m_barrelTfInputTag = iConfig.getParameter<edm::InputTag>("barrelTFInput");
  m_overlapTfInputTag = iConfig.getParameter<edm::InputTag>("overlapTFInput");
  m_forwardTfInputTag = iConfig.getParameter<edm::InputTag>("forwardTFInput");
  m_trigTowerTag = iConfig.getParameter<edm::InputTag>("triggerTowerInput");

  // m_barrelTfInputToken = consumes<MicroGMTConfiguration::InputCollection>(m_barrelTfInputTag);
  // m_overlapTfInputToken = consumes<MicroGMTConfiguration::InputCollection>(m_overlapTfInputTag);
  // m_forwardTfInputToken = consumes<MicroGMTConfiguration::InputCollection>(m_forwardTfInputTag);

  //register your products
  produces<MuonBxCollection>();
  produces<MuonBxCollection>("imdMuonsBMTF");
  produces<MuonBxCollection>("imdMuonsEMTFPos");
  produces<MuonBxCollection>("imdMuonsEMTFNeg");
  produces<MuonBxCollection>("imdMuonsOMTFPos");
  produces<MuonBxCollection>("imdMuonsOMTFNeg");

  

}

l1t::MicroGMTEmulator::~MicroGMTEmulator()
{
  m_debugOut.close();
}


//
// member functions
//



// ------------ method called to produce the data  ------------
void
l1t::MicroGMTEmulator::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;
  std::auto_ptr<MuonBxCollection> outMuons (new MuonBxCollection());
  std::auto_ptr<MuonBxCollection> imdMuonsBMTF (new MuonBxCollection());
  std::auto_ptr<MuonBxCollection> imdMuonsEMTFPos (new MuonBxCollection());
  std::auto_ptr<MuonBxCollection> imdMuonsEMTFNeg (new MuonBxCollection());
  std::auto_ptr<MuonBxCollection> imdMuonsOMTFPos (new MuonBxCollection());
  std::auto_ptr<MuonBxCollection> imdMuonsOMTFNeg (new MuonBxCollection());

  Handle<MicroGMTConfiguration::InputCollection> bmtfMuons;
  Handle<MicroGMTConfiguration::InputCollection> emtfMuons;
  Handle<MicroGMTConfiguration::InputCollection> omtfMuons;
  Handle<MicroGMTConfiguration::CaloInputCollection> trigTowers;

  // iEvent.getByToken(m_barrelTfInputToken, bmtfMuons);
  iEvent.getByLabel(m_barrelTfInputTag, bmtfMuons);
  iEvent.getByLabel(m_forwardTfInputTag, emtfMuons);
  iEvent.getByLabel(m_overlapTfInputTag, omtfMuons);
  iEvent.getByLabel(m_trigTowerTag, trigTowers);
  
  m_isolationUnit.setTowerSums(*trigTowers);
  MicroGMTConfiguration::InterMuonList internMuonsBmtf;
  MicroGMTConfiguration::InterMuonList internMuonsEmtfPos;
  MicroGMTConfiguration::InterMuonList internMuonsEmtfNeg;
  MicroGMTConfiguration::InterMuonList internMuonsOmtfPos;
  MicroGMTConfiguration::InterMuonList internMuonsOmtfNeg;


  // These wedges contain shared pointers to the ones in the InterMuonList
  L1TGMTInternalWedges omtfNegWedges;
  L1TGMTInternalWedges bmtfWedges;
  L1TGMTInternalWedges emtfPosWedges;
  L1TGMTInternalWedges emtfNegWedges;
  L1TGMTInternalWedges omtfPosWedges;

  // this converts the InputMuon type to the InternalMuon type and splits them into 
  // positive / negative eta collections necessary as LUTs may differ for pos / neg.
  convertMuons(*bmtfMuons, internMuonsBmtf, bmtfWedges);
  splitAndConvertMuons(*emtfMuons, internMuonsEmtfPos, internMuonsEmtfNeg, emtfPosWedges, emtfNegWedges);
  splitAndConvertMuons(*omtfMuons, internMuonsOmtfPos, internMuonsOmtfNeg, omtfPosWedges, omtfNegWedges);

  // cancel out within the track finders:
  m_cancelOutUnit.setCancelOutBits(bmtfWedges, tftype::bmtf, cancelmode::coordinate);
  m_cancelOutUnit.setCancelOutBits(omtfPosWedges, tftype::omtf_pos, cancelmode::coordinate);
  m_cancelOutUnit.setCancelOutBits(omtfNegWedges, tftype::omtf_neg, cancelmode::coordinate);
  // cancel-out for endcap will be done in the sorter
  // m_cancelOutUnit.setCancelOutBits(emtfPosWedges, tftype::emtf_pos);
  // m_cancelOutUnit.setCancelOutBits(emtfNegWedges, tftype::emtf_neg);

  // cancel out between track finder acceptance overlaps:
  m_cancelOutUnit.setCancelOutBitsOverlapBarrel(omtfPosWedges, bmtfWedges, cancelmode::coordinate);
  m_cancelOutUnit.setCancelOutBitsOverlapBarrel(omtfNegWedges, bmtfWedges, cancelmode::coordinate);
  m_cancelOutUnit.setCancelOutBitsOverlapEndcap(omtfPosWedges, emtfPosWedges, cancelmode::coordinate);
  m_cancelOutUnit.setCancelOutBitsOverlapEndcap(omtfNegWedges, emtfNegWedges, cancelmode::coordinate);
  
  m_isolationUnit.extrapolateMuons(internMuonsBmtf);
  m_isolationUnit.extrapolateMuons(internMuonsEmtfNeg);
  m_isolationUnit.extrapolateMuons(internMuonsEmtfPos);
  m_isolationUnit.extrapolateMuons(internMuonsOmtfNeg);
  m_isolationUnit.extrapolateMuons(internMuonsOmtfPos);

  // the rank calculated here is used in the sort below
  calculateRank(internMuonsBmtf);
  calculateRank(internMuonsEmtfNeg);
  calculateRank(internMuonsEmtfPos);
  calculateRank(internMuonsOmtfNeg);
  calculateRank(internMuonsOmtfPos);
  
  // The sort function both sorts and removes all but best "nSurvivors"
  sortMuons(internMuonsBmtf, 8);
  sortMuons(internMuonsOmtfPos, 4);
  sortMuons(internMuonsOmtfNeg, 4);
  sortMuons(internMuonsEmtfPos, 4);
  sortMuons(internMuonsEmtfNeg, 4);

  // This combines the 5 streams into one InternalMuon collection for 
  // the final global sort.
  MicroGMTConfiguration::InterMuonList internalMuons;
  addMuonsToCollections(internMuonsEmtfPos, internalMuons, imdMuonsEMTFPos);
  addMuonsToCollections(internMuonsOmtfPos, internalMuons, imdMuonsOMTFPos);
  addMuonsToCollections(internMuonsBmtf, internalMuons, imdMuonsBMTF);
  addMuonsToCollections(internMuonsOmtfNeg, internalMuons, imdMuonsOMTFNeg);
  addMuonsToCollections(internMuonsEmtfNeg, internalMuons, imdMuonsEMTFNeg);
  
  // sort internal muons and delete all but best 8
  sortMuons(internalMuons, 8);

  m_isolationUnit.isolatePreSummed(internalMuons);
  // copy muons to output collection...
  for (const auto& mu : internalMuons) {
    if (mu->hwPt() > 0) {
      math::PtEtaPhiMLorentzVector vec{mu->hwPt()*0.5, mu->hwEta()*0.010875, mu->hwGlobalPhi()*0.010908, 0.0};
      int iso = mu->hwAbsIso() + (mu->hwRelIso() << 1);
      // FIXME: once we debugged the change global -> local: Change hwLocalPhi -> hwGlobalPhi to test offsets
      Muon outMu{vec, mu->hwPt(), mu->hwEta(), mu->hwGlobalPhi(), mu->hwQual(), mu->hwSign(), mu->hwSignValid(), iso, 0, true, mu->hwIsoSum(), mu->hwDPhi(), mu->hwDEta(), mu->hwRank()};
      m_debugOut << mu->hwCaloPhi() << " " << mu->hwCaloEta() << std::endl;
      outMuons->push_back(0, outMu);
    }
  }
  
  iEvent.put(outMuons);
  iEvent.put(imdMuonsBMTF, "imdMuonsBMTF");
  iEvent.put(imdMuonsEMTFPos, "imdMuonsEMTFPos");
  iEvent.put(imdMuonsEMTFNeg, "imdMuonsEMTFNeg");
  iEvent.put(imdMuonsOMTFPos, "imdMuonsOMTFPos");
  iEvent.put(imdMuonsOMTFNeg, "imdMuonsOMTFNeg");
}


bool 
l1t::MicroGMTEmulator::compareMuons(const std::shared_ptr<MicroGMTConfiguration::InterMuon>& mu1, 
                                    const std::shared_ptr<MicroGMTConfiguration::InterMuon>& mu2) {
  return (mu1->hwWins() > mu2->hwWins());
}

void 
l1t::MicroGMTEmulator::sortMuons(MicroGMTConfiguration::InterMuonList& muons, unsigned nSurvivors) const {
  MicroGMTConfiguration::InterMuonList::iterator mu1;
  // reset from previous sort stage
  for (mu1 = muons.begin(); mu1 != muons.end(); ++mu1) {
    (*mu1)->setHwWins(0);
  }
  
  for (mu1 = muons.begin(); mu1 != muons.end(); ++mu1) {
    auto mu2 = mu1;
    mu2++;
    for ( ; mu2 != muons.end(); ++mu2) {
      if ((*mu1)->hwRank() >= (*mu2)->hwRank() && (*mu1)->hwCancelBit() != 1) {
        (*mu1)->increaseWins();
      } else if ((*mu2)->hwCancelBit() != 1) {
        (*mu2)->increaseWins();
      }
    }
  }

  size_t nMuonsBefore = muons.size();
  mu1 = muons.begin();
  int minWins = nMuonsBefore-nSurvivors;
  
  // remove all muons that were cancelled or that do not have sufficient rank
  // (reduces the container size to nSurvivors)
  while (mu1 != muons.end()) {
    if ((*mu1)->hwWins() < minWins || (*mu1)->hwCancelBit() == 1) {
      muons.erase(mu1);
    }
    ++mu1;
  }
  muons.sort(l1t::MicroGMTEmulator::compareMuons);
}



void 
l1t::MicroGMTEmulator::calculateRank(MicroGMTConfiguration::InterMuonList& muons) const 
{
  for (auto& mu1 : muons) {
    int rank = m_rankPtQualityLUT.lookup(mu1->hwPt(), mu1->hwQual());
    mu1->setHwRank(rank);
  }
}


void 
l1t::MicroGMTEmulator::addMuonsToCollections(MicroGMTConfiguration::InterMuonList& coll, 
                                             MicroGMTConfiguration::InterMuonList& interout, 
                                             std::auto_ptr<MuonBxCollection>& out) const 
{
  for (auto& mu : coll) { 
    interout.push_back(mu);
    ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > vec{};
    // FIXME: once we debugged the change global -> local: Change hwLocalPhi -> hwGlobalPhi to test offsets
    Muon outMu{vec, mu->hwPt(), mu->hwEta(), mu->hwGlobalPhi(), mu->hwQual(), mu->hwSign(), mu->hwSignValid(), -1, 0, true, -1, mu->hwDPhi(), mu->hwDEta(), mu->hwRank()};
    out->push_back(0, outMu);
  }
}

void
l1t::MicroGMTEmulator::splitAndConvertMuons(const MicroGMTConfiguration::InputCollection& in, 
                                            MicroGMTConfiguration::InterMuonList& out_pos, 
                                            MicroGMTConfiguration::InterMuonList& out_neg,
                                            L1TGMTInternalWedges& wedges_pos, 
                                            L1TGMTInternalWedges& wedges_neg) const
{
  // initialize the wedge collections:
  for (int i = 1; i <= 6; ++i) {
    wedges_pos[i] = std::vector<std::shared_ptr<L1TGMTInternalMuon>>();
    wedges_pos[i].reserve(3);
    wedges_neg[i] = std::vector<std::shared_ptr<L1TGMTInternalMuon>>();
    wedges_neg[i].reserve(3);
  }
  for (const auto& mu : in) {
    if(mu.hwEta() > 0) {
      std::shared_ptr<L1TGMTInternalMuon> out = std::make_shared<L1TGMTInternalMuon>(mu);
      out_pos.push_back(out);
      wedges_pos[mu.processor()].push_back(out);
    } else {
      std::shared_ptr<L1TGMTInternalMuon> out = std::make_shared<L1TGMTInternalMuon>(mu);
      out_neg.emplace_back(out);
      wedges_neg[mu.processor()].push_back(out);
    }
  }
}
        
void 
l1t::MicroGMTEmulator::convertMuons(const MicroGMTConfiguration::InputCollection& in, 
                                    MicroGMTConfiguration::InterMuonList& out, 
                                    L1TGMTInternalWedges& wedges) const
{
  // initialize the wedge collection:
  for (int i = 1; i <= 12; ++i) {
    wedges[i] = std::vector<std::shared_ptr<L1TGMTInternalMuon>>();
    wedges[i].reserve(3);
  }
  for (const auto& mu : in) {
    std::shared_ptr<L1TGMTInternalMuon> outMu = std::make_shared<L1TGMTInternalMuon>(mu); 
    out.emplace_back(outMu);
    wedges[mu.processor()].push_back(outMu);
  }
}

// ------------ method called once each job just before starting event loop  ------------
void 
l1t::MicroGMTEmulator::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
l1t::MicroGMTEmulator::endJob() {
}

// ------------ method called when starting to processes a run  ------------
void 
l1t::MicroGMTEmulator::beginRun(edm::Run&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a run  ------------
void 
l1t::MicroGMTEmulator::endRun(edm::Run&, edm::EventSetup const&)
{
}

// ------------ method called when starting to processes a luminosity block  ------------
void 
l1t::MicroGMTEmulator::beginLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&)
{
}

// ------------ method called when ending the processing of a luminosity block  ------------
void 
l1t::MicroGMTEmulator::endLuminosityBlock(edm::LuminosityBlock&, edm::EventSetup const&)
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
l1t::MicroGMTEmulator::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(l1t::MicroGMTEmulator);
