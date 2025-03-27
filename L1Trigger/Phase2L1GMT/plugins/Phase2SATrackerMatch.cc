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
  void beginJob() ;
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

  std::cout << "SAMuons variables: "  << std::endl;
  for (const auto& samuon : *samuons) {
    std::cout<< "SAMuon " << std::endl;
    std::cout << "SAMuon pt: " << samuon.pt() << std::endl;
    std::cout << "SAMuon eta: " << samuon.eta() << std::endl;
    std::cout << "SAMuon phi: " << samuon.phi() << std::endl;
    std::cout << "SAMuon hwPt: " << samuon.hwPt() << std::endl;
    std::cout << "SAMuon hwEta: " << samuon.hwEta() << std::endl;
    std::cout << "SAMuon hwPhi: " << samuon.hwPhi() << std::endl;

  }

  // Process each SAMuon
  if (verbose_) {
    for (const auto& samuon : *samuons) {
      if(verbose_) std::cout << "SAMuon pt before matching: " << samuon.pt() << std::endl;
      for (const auto& stub : samuon.stubs()) {
        if (verbose_) {
          std::cout << " SA Stub layer: " << stub->tfLayer() << " bx: " << stub->bxNum() << std::endl;
          std::cout << " eta1: " << stub->eta1() << std::endl;
          std::cout << " eta2: " << stub->eta2() << std::endl;
          std::cout << " coord1: " << stub->coord1() << std::endl;
          std::cout << " coord2: " << stub->coord2() << std::endl;
        }
      }
    }

    // Process each TrackerMuon
    for (const auto& trackerMuon : *trackerMuons) {
      if (verbose_) std::cout << "TrackerMuon: " << trackerMuon.pt() << std::endl;
      for (const auto& stub : trackerMuon.stubs()) {
        if (verbose_) {
          std::cout << " Tracker Stub layer: " << stub->tfLayer() << " bx: " << stub->bxNum() << std::endl;
          std::cout << " eta1: " << stub->eta1() << std::endl;
          std::cout << " eta2: " << stub->eta2() << std::endl;
          std::cout << " coord1: " << stub->coord1() << std::endl;
          std::cout << " coord2: " << stub->coord2() << std::endl;
        }
      }
    }
  }

  // Check how many stubs are in common between the two collections by checking variables eta1, eta2, coord1, coord2

    for (auto& samuon : *samuons) {
        int commonStubCount = 0;
        std::vector<int> commonQualityVector;
        for (auto& trackerMuon : *trackerMuons) {
            for ( auto& samuonStub : samuon.stubs()) {
                for ( auto& trackerMuonStub : trackerMuon.stubs()) {
                    bool sameEta1 = samuonStub->eta1() == trackerMuonStub->eta1();
                    bool sameEta2 = samuonStub->eta2() == trackerMuonStub->eta2();
                    bool sameCoord1 = samuonStub->coord1() == trackerMuonStub->coord1();
                    bool sameCoord2 = samuonStub->coord2() == trackerMuonStub->coord2();
                    int commonQuality = sameEta1 + sameEta2 + sameCoord1 + sameCoord2;
                    if (commonQuality > 0 && samuonStub->tfLayer() == trackerMuonStub->tfLayer() && samuonStub->bxNum() == trackerMuonStub->bxNum()) {
                        commonStubCount++;
                        commonQualityVector.push_back(commonQuality);
                    }
                }
            }
        }
        if (verbose_) {
            std::cout << "SAMuon pt after matching: " << samuon.pt() << std::endl;
            int stubIndex = 1;
            for (const auto& quality : commonQualityVector) {
                std::cout << "Stub Index: " << stubIndex << ", Quality: " << quality << std::endl;
                stubIndex++;
            }
        }

        int totalQualitySum = 0;
        for (const auto& quality : commonQualityVector) {
            totalQualitySum += quality;
        }

        l1t::SAMuon newSAmuon = {samuon};
        newSAmuon.setMatchedStubCount(commonStubCount);
        newSAmuon.setTotalStubQuality(totalQualitySum);
        samuonsWithCommonStubInfo.push_back(newSAmuon);

        if (verbose_) {
            std::cout << "Matched stub count: " << newSAmuon.matchedStubCount() << std::endl;
            std::cout << "Total stub quality: " << newSAmuon.totalStubQuality() << std::endl;
        }
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