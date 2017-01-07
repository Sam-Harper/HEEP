#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/stream/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/Ptr.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/PatCandidates/interface/VIDCutFlowResult.h"

//these header files give us easy to use shortcuts for which cut
//corresponds to which which cutnr
//this is fixed for a given ID (and can be different for each ID)
//hence its hard coded
//also these headerfiles are intentionally completely standalone 
//so you can easily include them in your analysis if you find them
//useful
#include "HEEP/VID/interface/CutNrs.h"
#include "HEEP/VID/interface/VIDCutCodes.h"

//**********************************************************
//
// class: HEEPV70PATExample
//
// author: Sam Harper (RAL)
//
// this class is meant to serve as an example of how to 
// access the HEEP V7.0 ID + new tracker isolation when
// they have been embeded into the pat::Electron
//
// these HEEPified electrons have the following added them
//
// userInt("heepElectronID_HEEPV70") : 
//     HEEP ID result, 0 = fail, 1 = pass
// userInt("heepElectronID_HEEPV70Bitmap") : 
//     HEEP ID result in detail, each bit corresponds to a cut: 0 = fail, 1 = pass
//     eg bit 0 is the et cut
// userInt("nrSatCrys") : 
//     number of saturated crystals in the 5x5 centred on seed crystal
// userFloat("trkPtIso") : 
//     new recalculated value of the track isolation
// userData<vid::CutFlowResult>("heepElectronID_HEEPV70") : 
//     the full vid::CutFlowResult with almost full information about 
//     which cuts passed/failed etc
//     contains all of over the above information except for nr of sat crys

class HEEPV70PATExample : public edm::stream::EDAnalyzer<> {

private:
 
  edm::EDGetTokenT<edm::View<pat::Electron> > eleToken_;
  
public:
  explicit HEEPV70PATExample(const edm::ParameterSet& iPara);
  virtual ~HEEPV70PATExample(){}
  
private:
  void analyze(const edm::Event& iEvent,const edm::EventSetup& iSetup) override;
};
  

HEEPV70PATExample::HEEPV70PATExample(const edm::ParameterSet& iPara)
{
  eleToken_=consumes<edm::View<pat::Electron> >(iPara.getParameter<edm::InputTag>("eles")); 
}

void HEEPV70PATExample::analyze(const edm::Event& iEvent,const edm::EventSetup& iSetup)
{
  edm::Handle<edm::View<pat::Electron> > eleHandle;
 
  iEvent.getByToken(eleToken_,eleHandle);
 

  for(auto& ele : *eleHandle){

    //first we are going to access this ignoring the vid::CutFlowResult

    //access new tracker isolation
    const float trkIso = ele.userFloat("trkPtIso");
    //access # saturated crystals in the 5x5
    const float nrSatCrys = ele.userInt("nrSatCrys");
    //access the HEEP ID pass / fail
    const bool heepID = ele.userInt("heepElectronID_HEEPV70");
    //access the detailed information on the HEEP ID
    const int heepIDBits = ele.userInt("heepElectronID_HEEPV70Bitmap");
  
    if(nrSatCrys!=0) std::cout <<"nrSatCrys "<<nrSatCrys<<std::endl;

    //the HEEP ID bits can be turn cuts on and off again
    //lets require just the et,sigmaIEtaIEta, E2X5/E5x5 and H/E

    //first, we need to know what which bits corrspond to which cuts
    //cutnrs::HEEPV70 defines an enum corresponding each cut to bit
    //VIDCutCodes uses this to give nice pass / fail functions
    using HEEPV70 = VIDCutCodes<cutnrs::HEEPV70>;
    //note, here we use a std::intialisation list (the {...}) to make 
    //vector of all the cuts we would like to pass
    const bool passEtShowerShapeHE = HEEPV70::pass(heepIDBits,{HEEPV70::ET,HEEPV70::SIGMAIETAIETA,HEEPV70::E2X5OVER5X5,HEEPV70::HADEM});

    //now lets require all cuts except tracker isolation
    const bool passN1TrkIso = HEEPV70::pass(heepIDBits,HEEPV70::TRKISO,HEEPV70::IGNORE);

    //now we are going to access all of the information above via the vid::CutFlowResult 
    //well except for the nrSatCrys

    //this will be null if its not present
    const vid::CutFlowResult* vidResult =  ele.userData<vid::CutFlowResult>("heepElectronID_HEEPV70");
    //how to check if everything passed:
    const bool heepIDVID = vidResult->cutFlowPassed();

    if(heepID!=heepIDVID) std::cout <<"error in VID HEEP ID result "<<std::endl;
    
    //how to get the track isolation from VID
    //note, this works for all cuts except E2x5/E5x5 although this is a feature which is
    //not often used so safer to use the standard accessors
    //trk isolation value is confirmed to be okay though
    const float trkIsoVID = vidResult->getValueCutUpon(HEEPV70::TRKISO);
    if(trkIso!=trkIsoVID) std::cout <<"error in VID trk isol "<<std::endl;
    
    //now lets do selective cuts like we did before
    const bool passEtShowerShapeHEVID = vidResult->getCutResultByIndex(HEEPV70::ET)
      && vidResult->getCutResultByIndex(HEEPV70::SIGMAIETAIETA) 
      && vidResult->getCutResultByIndex(HEEPV70::E2X5OVER5X5)
      && vidResult->getCutResultByIndex(HEEPV70::HADEM);

    //now for track isolation
    //now this may not be the fastest function as it makes a new cut flow...
    const bool passN1TrkIsoVID = vidResult->getCutFlowResultMasking(HEEPV70::TRKISO).cutFlowPassed();
    
    if(passEtShowerShapeHE != passEtShowerShapeHEVID) std::cout <<"error in VID showershape cuts"<<std::endl;
    if(passN1TrkIso != passN1TrkIsoVID) std::cout <<"error in VID trk iso cuts"<<std::endl;
   

  }
  
 
}

DEFINE_FWK_MODULE(HEEPV70PATExample);
 
