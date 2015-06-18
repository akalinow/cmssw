///////////////////////////////////////////////////////////////
// Upgraded Encdap Muon Track Finding Algorithm		    	//
//							   								//
// Info: A human-readable version of the firmware based     //
//       track finding algorithm which will be implemented  //
//       in the upgraded endcaps of CMS. DT and RPC inputs  //
//	     are not considered in this algorithm.      		//
//								   							//
// Author: M. Carver (UF)				    				//
//////////////////////////////////////////////////////////////


#include "L1Trigger/L1EndcapMuonTrackFinder/plugins/L1TMuonUpgradedTrackFinder.h"
#include "L1Trigger/CSCCommonTrigger/interface/CSCPatternLUT.h"
#include "L1Trigger/CSCTrackFinder/test/src/RefTrack.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "PrimitiveConverter.h"
#include "BXAnalyzer.h"
#include "ZoneCreation.h"
#include "PatternRecognition.h"
#include "SortSector.h"
#include "Matching.h"
#include "Deltas.h"
#include "BestTracks.h"
#include "PtAssignment.h"


using namespace L1TMuon;


L1TMuonUpgradedTrackFinder::L1TMuonUpgradedTrackFinder(const PSet& p) {
  if( (_dogen = p.getUntrackedParameter<bool>("doGen",false)) ) {
    _geninput = p.getUntrackedParameter<edm::InputTag>("genSrc");
  }
  _tpinputs = p.getParameter<std::vector<edm::InputTag> >("primitiveSrcs");
  _convTrkInputs = 
    p.getParameter<std::vector<edm::InputTag> >("converterSrcs");
    
    LUTparam = p.getParameter<edm::ParameterSet>("lutParam");
    
    produces<L1TMuon::InternalTrackCollection> ("DataITC").setBranchAlias("DataITC");
}


void L1TMuonUpgradedTrackFinder::produce(edm::Event& ev, 
			       const edm::EventSetup& es) {
				   
  bool verbose = false;
			       
 		
  std::cout<<"Start Upgraded Track Finder Producer::::: event = "<<ev.id().event()<<"\n\n";
  
  //fprintf (write,"12345\n"); //<-- part of printing text file to send verilog code, not needed if George's package is included
  
  
  std::auto_ptr<L1TMuon::InternalTrackCollection> FoundTracks (new L1TMuon::InternalTrackCollection);
  
  std::vector<BTrack> PTracks[12];
 
  std::vector<TriggerPrimitiveRef> tester;
  //std::vector<InternalTrack> FoundTracks;
  
  //////////////////////////////////////////////
  ////////// Get Generated Muons ///////////////
  //////////////////////////////////////////////
  
  edm::Handle<std::vector<reco::GenParticle>> GenMuons;
  std::vector<reco::GenParticle>::const_iterator GI;
  ev.getByLabel("genParticles",GenMuons);
  reco::GenParticle GeneratorMuon;
  for(GI=GenMuons->begin();GI!=GenMuons->end();GI++){
  	
	const reco::GenParticle GenMuon = *GI;
	GeneratorMuon = GenMuon;
	double pt = GenMuon.pt(), eta = GenMuon.eta(), phi = GenMuon.phi(), mass = GenMuon.mass();
	int charge = GenMuon.charge();
	
	std::cout<<"Gen Particle Info::::\nPt = "<<pt<<", phi = "<<phi<<", eta = "<<eta<<", mass = "<<mass<<" and charge = "<<charge<<"\n\n";
		
  }
  
  
  //////////////////////////////////////////////
  ///////// Get Trigger Primitives /////////////  Retrieve TriggerPrimitives from the event record
  //////////////////////////////////////////////
  
  auto tpsrc = _tpinputs.cbegin();
  auto tpend = _tpinputs.cend();
  for( ; tpsrc != tpend; ++tpsrc ) {
    edm::Handle<TriggerPrimitiveCollection> tps;
    ev.getByLabel(*tpsrc,tps);
    auto tp = tps->cbegin();
    auto tpend = tps->cend();

    for( ; tp != tpend; ++tp ) {
      if(tp->subsystem() == 1)
      {
		TriggerPrimitiveRef tpref(tps,tp - tps -> cbegin());
		
		tester.push_back(tpref);
		
		std::cout<<"\ntrigger prim found station:"<<tp->detId<CSCDetId>().station()<<std::endl;
      }
 
     }    
   }
  std::vector<ConvertedHit> CHits[12];
  MatchingOutput MO[12];
 
for(int SectIndex=0;SectIndex<12;SectIndex++){//perform TF on all 12 sectors

 
 
  //////////////////////////////////////////////////////  Input is raw hit information from 
  ///////////////// TP Conversion //////////////////////  Output is vector of Converted Hits
  //////////////////////////////////////////////////////


 	std::vector<ConvertedHit> ConvHits = PrimConv(tester,SectIndex);
	CHits[SectIndex] = ConvHits;
 
  
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
////////////////////////////////print values for input into Alex's emulator code/////////////////////////////////////////////////////
	//for(std::vector<ConvertedHit>::iterator h = ConvHits.begin();h != ConvHits.end();h++){
	
		//if((h->Id()) > 9){h->SetId(h->Id() - 9);h->SetStrip(h->Strip() + 128);}	
		//fprintf (write,"0	1	1 	%d	%d\n",h->Sub(),h->Station());
		//fprintf (write,"1	%d	%d 	%d\n",h->Quality(),h->Pattern(),h->Wire());
		//fprintf (write,"%d	0	%d\n",h->Id(),h->Strip());	
	//}
////////////////////////////////print values for input into Alex's emulator code/////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  


 //////////////////////////////////////////////////////
 //////////////////////////////////////////////////////  Takes the vector of converted hits and groups into 3 groups of hits
 ////////////////////// BX Grouper ////////////////////  which are 3 BX's wide. Effectively looking 2 BX's into the future and 
 //////////////////////////////////////////////////////  past from the central BX, this analyzes a total of 5 BX's.
 //////////////////////////////////////////////////////
 
 
 std::vector<std::vector<ConvertedHit>> GroupedHits = GroupBX(ConvHits);
 
 
////////////////////////////////////////////////////////  Creates a zone for each of the three groups created in the BX Grouper module.
////////// Creat Zones for pattern Recognition /////////  The output of this module not only contains the zones but also the 
////////////////////////////////////////////////////////  reference back to the TriggerPrimitives that went into making them.
	
 std::vector<ZonesOutput> Zout = Zones(GroupedHits);
   

  ///////////////////////////////
  ///// Pattern Recognition /////  Applies pattern recognition logic on each of the 3 BX groups and assigns a quality to each keystrip in the zone.
  ///// & quality assinment /////  The delete duplicate patterns function looks at the 3 BX groups and deletes duplicate patterns found from the
  ///////////////////////////////  same hits. This is where the BX analysis ends; Only 1 list of found patterns is given to the next module.
  

  std::vector<PatternOutput> Pout = Patterns(Zout);
  
  PatternOutput Test = DeleteDuplicatePatterns(Pout);
 
  PrintQuality(Test.detected);
 

  ///////////////////////////////
  //////Sector Sorting/////////// Sorts through the patterns found in each zone and selects the best three per zone to send to the next module.
  ///////Finding 3 Best Pattern// 
  ///////////////////////////////
  
  
  SortingOutput Sout = SortSect(Test);
	
 
  //////////////////////////////////
  ///////// Match ph patterns ////// Loops over each sorted pattern and then loops over all possible triggerprimitives which could have made the pattern
  ////// to segment inputs ///////// and matches the associated full precision triggerprimitives to the detected pattern. 
  //////////////////////////////////   
      

  MatchingOutput Mout = PhiMatching(Sout);
  MO[SectIndex] = Mout;

  /////////////////////////////////
  //////// Calculate delta //////// Once we have matched the hits we calculate the delta phi and theta between all 
  ////////    ph and th    //////// stations present. 
  /////////////////////////////////
  
  
 std::vector<std::vector<DeltaOutput>> Dout = CalcDeltas(Mout);////
 

  /////////////////////////////////
  /////// Sorts and gives /////////  Loops over all of the found tracks(looking across zones) and selects the best three per sector. 
  ////// Best 3 tracks/sector /////  Here ghost busting is done to delete tracks which are comprised of the same associated stubs. 
  /////////////////////////////////  
  
  
  std::vector<BTrack> Bout = BestTracks(Dout);
   PTracks[SectIndex] = Bout;
   
  
  	
  }
 
 ////////////////////////////////////
 //// Ghost Cancellation between ////not done correctly
 //////////   sectors     ///////////
 ////////////////////////////////////
 
 for(int i1=0;i1<36;i1++){
 
 	for(int i2=i1+1;i2<36;i2++){
		
		int sec[2] = {i1/3,i2/3};
		int num[2] = {i1%3,i2%3};
		
		bool wrap = ((sec[0] == 0 || sec[0] == 6) && (fabs(sec[0] - sec[1]) == 5));
		bool same = (sec[0] == sec[1]);
		bool toofar = (fabs(sec[0] - sec[1]) > 1);

		if((same || toofar) && !wrap)//if same chamber or more than one chamber away dont do. !wrap allows comparison between sectors 0&5 and 6&11
			continue;
			
		if(!PTracks[sec[0]][num[0]].AHits.size() || !PTracks[sec[1]][num[1]].AHits.size())
			continue;
		
		int sh_seg = 0;
		
		if(verbose) std::cout<<"\nComparing adjacent sectors\n";

		for(int sta=0;sta<4;sta++){//the part which is done incorrectly
		
			if((PTracks[sec[0]][num[0]].AHits[sta].Phi() == -999) || (PTracks[sec[1]][num[1]].AHits[sta].Phi() == -999))
				continue;
				
		
			if((PTracks[sec[0]][num[0]].AHits[sta].Id() == PTracks[sec[1]][num[1]].AHits[sta].Id())
				&& (PTracks[sec[0]][num[0]].AHits[sta].Strip() == PTracks[sec[1]][num[1]].AHits[sta].Strip())
				&& (PTracks[sec[0]][num[0]].AHits[sta].Wire() == PTracks[sec[1]][num[1]].AHits[sta].Wire())){
				
				sh_seg++;
			}
			
		}
		
		if(sh_seg){//if any segments are shared delete the track with lower rank
			
			BTrack tmp;//default null track to replace ghost with
			
			if(PTracks[sec[0]][num[0]].winner.Rank() >= PTracks[sec[1]][num[1]].winner.Rank())
				PTracks[sec[1]][num[1]] = tmp;
			else
				PTracks[sec[0]][num[0]] = tmp;
		
		}
 	}
 }
 
 
 ////////////////////////////////////
 /// Sorting through all sectors ////
 ///   to find 4 best muons      ////
 ////////////////////////////////////
 
 
 BTrack FourBest[4];//ok
 std::vector<BTrack> PTemp[12] = PTracks;
 int windex[4] = {-1,-1,-1,-1};
 
 
 
 for(int i=0;i<4;i++){
 
 	for(int j=0;j<36;j++){
 
		
			if(!PTemp[j/3][j%3].phi)//no track
				continue;
			
			if((windex[0] == j) || (windex[1] == j) || (windex[2] == j) || (windex[3] == j))//already picked
				continue;
		
			if(PTracks[j/3][j%3].winner.Rank() > FourBest[i].winner.Rank()){
			
				FourBest[i] = PTemp[j/3][j%3];
				windex[i] = j;
				
			} 

 	}
}
  ///////////////////////////////////
  /// Make Internal track if ////////
  /////// tracks are found //////////
  ///////////////////////////////////

  //bool epir = false;
  
  for(int fbest=0;fbest<4;fbest++){
  
  	if(FourBest[fbest].phi){
	
		//epir = true;
	
		InternalTrack tempTrack;
  		tempTrack.setType(2); 
	
		tempTrack.phi = FourBest[fbest].phi;
		tempTrack.theta = FourBest[fbest].theta;
		tempTrack.rank = FourBest[fbest].winner.Rank();
		tempTrack.deltas = FourBest[fbest].deltas;
		std::vector<int> ps, ts;
		
		for(std::vector<ConvertedHit>::iterator A = FourBest[fbest].AHits.begin();A != FourBest[fbest].AHits.end();A++){
		
			if(A->Phi() != -999){
			
				tempTrack.addStub(A->TP());
				ps.push_back(A->Phi());
				ts.push_back(A->Theta());
				//std::cout<<"Q: "<<A->Quality()<<", keywire: "<<A->Wire()<<", strip: "<<A->Strip()<<std::endl;
			}
			
		}
		tempTrack.phis = ps;
		tempTrack.thetas = ts;
		
		std::cout<<"\n\nTrack "<<fbest<<": ";
		//CalculatePt(tempTrack);
		tempTrack.pt = CalculatePt(tempTrack);
		std::cout<<"XML pT = "<<tempTrack.pt<<"\n";
		FoundTracks->push_back(tempTrack);
		std::cout<<"\n\n";
	}
  }
  
 
 //  std::cout<<"Begin Put function\n\n";
ev.put( FoundTracks, "DataITC");
  std::cout<<"End Upgraded Track Finder Prducer:::::::::::::::::::::::::::\n:::::::::::::::::::::::::::::::::::::::::::::::::\n\n";

}//analyzer

void L1TMuonUpgradedTrackFinder::beginJob()
{

	//std::cout<<"Begin TextDump Prducer:::::::::::::::::::::::::::\n:::::::::::::::::::::::::::::::::::::::::::::::::\n\n";
	///////////////////////////
	////// Histogram //////////
	////// Declaration ////////
	///////////////////////////
	
	TFileDirectory dir = histofile->mkdir("1");//

	
	///////////////////////////
	/////// Output ////////////
	///// Text Files //////////
	///////////////////////////
	
	
	write = fopen ("zone0.txt","w");

	
	
	
}
void L1TMuonUpgradedTrackFinder::endJob()
{

	fclose (write);
	
	
	
		
	std::cout<<"\nTHE END"<<std::endl;
	
}
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(L1TMuonUpgradedTrackFinder);
