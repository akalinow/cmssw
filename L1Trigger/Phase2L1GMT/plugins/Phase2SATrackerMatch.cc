#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "DataFormats/L1TMuonPhase2/interface/TrackerMuon.h"
#include "DataFormats/L1TMuonPhase2/interface/SAMuon.h"
#include <iostream>
#include <vector>



class Phase2SATrackerMatch : public edm::stream::EDProducer<> {
public:
  explicit Phase2SATrackerMatch(const edm::ParameterSet&);
  ~Phase2SATrackerMatch() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginJob();

  int findBestTrackerMuon(const l1t::SAMuon & samuon, 
                          const std::vector<l1t::TrackerMuon>& trackerMuons) const;

  void produce(edm::Event&, const edm::EventSetup&) override;
  void endJob() ;
  bool verbose_;

  edm::EDGetTokenT<std::vector<l1t::SAMuon>> samuonToken_;
  edm::EDGetTokenT<std::vector<l1t::TrackerMuon>> trackerMuonToken_;
};

// Constructor: Initializes the Phase2SATrackerMatch class
Phase2SATrackerMatch::Phase2SATrackerMatch(const edm::ParameterSet& iConfig)
    : verbose_(iConfig.getUntrackedParameter<bool>("verbose", false)),
      samuonToken_(consumes<std::vector<l1t::SAMuon>>(iConfig.getParameter<edm::InputTag>("samuons"))),
      trackerMuonToken_(consumes<std::vector<l1t::TrackerMuon>>(iConfig.getParameter<edm::InputTag>("trackerMuons"))) {
  // Register the product
  produces<std::vector<l1t::SAMuon>>("SAMuonsWithCommonStubInfo").setBranchAlias("SAMuonsWithCommonStubInfo");
}

// Destructor: Cleans up any resources used by the Phase2SATrackerMatch class
Phase2SATrackerMatch::~Phase2SATrackerMatch() {}

// Called once at the beginning of the job
void Phase2SATrackerMatch::beginJob() {}

int Phase2SATrackerMatch::findBestTrackerMuon(const l1t::SAMuon & samuon, 
                                              const std::vector<l1t::TrackerMuon>& trackerMuons) const {

  LogDebug("SAMuon") << "Phase2SATrackerMatch::findBestTrackerMuon called with SAMuon: " << std::endl;
  samuon.print();
  LogDebug("SAMuon") << "SAMuon stubs: " << std::endl;
  for (auto& samuonStub : samuon.stubs()) samuonStub->print();
  LogDebug("SAMuon") << "SAMuon stubs: END" << std::endl;
  LogDebug("SAMuon") << "Number of TrackerMuons: " << trackerMuons.size() << std::endl;
  
  int bestMatchCount = -1;
  
  for (auto& trackerMuon : trackerMuons) {

    int commonStubCount = 0;
    if(trackerMuon.hwQual()<4) continue; //skip non-zero bx and low quality TrackerMuons
    
    for (auto& trackerMuonStub : trackerMuon.stubs()) trackerMuonStub->print();
    
    for (auto& samuonStub : samuon.stubs()){
        bool stubMatched = false;
        for (auto& trackerMuonStub : trackerMuon.stubs()) {

            bool tfLayerMatch = (samuonStub->tfLayer() == trackerMuonStub->tfLayer());
            bool bxNumMatch = (samuonStub->bxNum() == trackerMuonStub->bxNum());
            bool typeMatch = (samuonStub->type() == trackerMuonStub->type());
            bool addressMatch = (samuonStub->address()>>2) == (trackerMuonStub->address()>>2); //ignore last 2 bits of address - stub id within chamber
            bool qualityMatch = true; //hybrid stubs have quality fixed to 3 (samuonStub->quality() == trackerMuonStub->quality());
            bool etaQualityMatch = (samuonStub->etaQuality() == trackerMuonStub->etaQuality());

            LogDebug("SAMuon") <<" SAM:" << (samuonStub->address()>>2)
                                <<" TK: " << (trackerMuonStub->address()>>2) 
                                << " bxNumMatch: " << bxNumMatch
                                << " tfLayerMatch: " << tfLayerMatch 
                                << " typeMatch: " << typeMatch
                                << " addressMatch: " << addressMatch
                                << " qualityMatch: " << qualityMatch
                                << " etaQualityMatch: " << etaQualityMatch
                                << std::endl;

            //stubMatched=(tfLayerMatch && addressMatch); //full match in HW. In HW we have access to tfLayer and address.
            stubMatched |= addressMatch; //Catch cases when SAM and TKMuons have stubs in different tfLayers.
            
            ///etaQuality:
            /// 0 = no eta coordinate
            /// 1 = eta1 coordinate only
            /// 2 = eta2 coordinates only
            /// 3 = both eta1 and eta2 coordinates

            ///type==0 && etaQuality==1 - DT/RPC with eta1, coord1
            ///type==0 && etaQuality==2 - RPC only, with eta2, coord2
            ///type==0 && etaQuality==3 - RPC+CSC with eta1, coord1, eta2, coord2

            ///type==1 && etaQuality==0 - DT coarse eta coordinate
            ///type==1 && etaQuality==1 - DT with single eta coordinate
            ///type==1 && etaQuality==3 - DT with two eta coordinates                              
        }
          commonStubCount += stubMatched;
    
      if(commonStubCount > bestMatchCount) bestMatchCount = commonStubCount;
    }
  }
    if(bestMatchCount<2){
      LogDebug("SAMuon")<<" LOW MATCH bestMatchCount: "<<bestMatchCount<<std::endl;
    }

    return bestMatchCount;
}
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
// Called for each event to produce the output
void Phase2SATrackerMatch::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  // Retrieve SAMuons from the event
  edm::Handle<std::vector<l1t::SAMuon>> samuons;
  iEvent.getByToken(samuonToken_, samuons);

  // Retrieve TrackerMuons from the event
  edm::Handle<std::vector<l1t::TrackerMuon>> trackerMuons;
  iEvent.getByToken(trackerMuonToken_, trackerMuons);

  // Vector to store SAMuons with common stub information
  std::vector<l1t::SAMuon> samuonsWithCommonStubInfo;

  int commonStubCount = 0;
  for (const auto& samuon : *samuons){
    commonStubCount = findBestTrackerMuon(samuon, *trackerMuons);
    l1t::SAMuon newSAmuon = {samuon};
    newSAmuon.setCommonStubCount(commonStubCount);
    newSAmuon.setTotalStubCount(samuon.stubs().size());
    samuonsWithCommonStubInfo.push_back(newSAmuon);
  }

  // Create and put the output collections
  auto outputSAMuonsWithCommonStubInfo = std::make_unique<std::vector<l1t::SAMuon>>(samuonsWithCommonStubInfo);
  iEvent.put(std::move(outputSAMuonsWithCommonStubInfo), "SAMuonsWithCommonStubInfo");
}

// Called once at the end of the job
void Phase2SATrackerMatch::endJob() {}

// Fills descriptions for the module
void Phase2SATrackerMatch::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("samuons", edm::InputTag("samuons"));
  desc.add<edm::InputTag>("trackerMuons", edm::InputTag("trackerMuons"));
  desc.addUntracked<bool>("verbose", false);
  descriptions.add("phase2SATrackerMatch", desc);
}

// Define this as a plug-in
DEFINE_FWK_MODULE(Phase2SATrackerMatch);