#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/stream/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/Ptr.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "SHarper/HEEPAnalyzer/interface/HEEPCutCodes.h"
#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/PatCandidates/interface/VIDCutFlowResult.h"

#include "TH1D.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "FWCore/ServiceRegistry/interface/Service.h"


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
  
    
    //now we are going to access all of the information above via the vid::CutFlowResult 
    //well except for the nrSatCrys

    //this will be null if its not present
    const vid::CutFlowResult* vidResult =  ele.userData<vid::CutFlowResult>("heepElectronID_HEEPV70");
    //how to check if everything passed:
    const bool heepIDVID = vidResult->cutFlowPassed();

    

  }
  
 
}

DEFINE_FWK_MODULE(HEEPV70PATExample);
 
