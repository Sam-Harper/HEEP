#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/stream/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/Ptr.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/EgammaCandidates/interface/GsfElectron.h"
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
// class: HEEPV70Example
//
// author: Sam Harper (RAL)
//
// this class is meant to serve as an example of how to 
// access the HEEP V7.0 ID + new tracker isolation 
// using the value maps calculated by VID
//
// a edm::ValueMap is a easy way of associating extra content
// to electrons (or other objects). You pass in an edm::Ref of edm::Ptr
// for the electron you want and you are returned the value stored for 
// that electron. Note, these references are collection specific, if you 
// make a new electron collection, the edm::Ref/edm::Ptrs of electrons in 
// that collection are not valid for that value map and so can not be used 
// with it.
//
// the following value maps are of interest
//   edm::ValueMap<vid::CutFlowResult>  egmGsfElectronIDs:heepElectronID-HEEPV70  :
//        the full VID result with lots of info
//   edm::ValueMap<unsigned int>  egmGsfElectronIDs:heepElectronID-HEEPV70Bitmap  :
//        bitmap of which cuts passed, bitNr X = cut X and 0=fail, 1 =pass
//   edm::ValueMap<bool>  egmGsfElectronIDs:heepElectronID-HEEPV70 
//        global pass/fail bool,  0=fail HEEPV70, 1 =pass HEEPV70
//   edm::ValueMap<int> heepIDVarValueMaps:eleNrSaturateIn5x5  :
//        specific to HEEEP ID (not technically part of VID but run by VID automatically)
//        nr of staturated crystals in 5x5 centred on the seed crystal
//   edm::ValueMap<int> heepIDVarValueMaps:eleNrSaturateIn5x5  :
//        specific to HEEEP ID (not technically part of VID but run by VID automatically)
//        the new tracker isolation used in the ID
//
//
// for reference: 
//   edm::ValueMap<unsigned int>  egmGsfElectronIDs:heepElectronID-HEEPV70  :
//        is which cut number was the last cut number passed
//        I do not consider this useful


//a struct to count the number of electrons passing/failing
//see https://twiki.cern.ch/twiki/bin/view/CMSPublic/FWMultithreadedFrameworkStreamModuleInterface
//if you dont understand why I'm doing this
namespace{
  struct NrPassFail {
    NrPassFail():nrPass(0),nrFail(0){}
    mutable std::atomic<int> nrPass;
    mutable std::atomic<int> nrFail;
  };
}

class HEEPV70Example : public edm::stream::EDAnalyzer<edm::GlobalCache<NrPassFail>> {

private:
 
  edm::EDGetTokenT<edm::View<reco::GsfElectron> > eleAODToken_;
  edm::EDGetTokenT<edm::View<reco::GsfElectron> > eleMiniAODToken_;
  edm::EDGetTokenT<edm::ValueMap<bool> > vidPassToken_;
  edm::EDGetTokenT<edm::ValueMap<unsigned int> > vidBitmapToken_;
  edm::EDGetTokenT<edm::ValueMap<vid::CutFlowResult> >  vidResultToken_;
  
  
  edm::EDGetTokenT<edm::ValueMap<int> > nrSatCrysMapToken_; 
  edm::EDGetTokenT<edm::ValueMap<float> > trkIsoMapToken_; 

  
public:
  explicit HEEPV70Example(const edm::ParameterSet& iPara,const NrPassFail*);
  virtual ~HEEPV70Example(){}
  
  static std::unique_ptr<NrPassFail> initializeGlobalCache(const edm::ParameterSet&) {
    return std::make_unique<NrPassFail>();
  }
  void analyze(const edm::Event& iEvent,const edm::EventSetup& iSetup) override;
  static void globalEndJob(const NrPassFail* nrPassFail) {
    std::cout <<"nr eles pass "<<nrPassFail->nrPass<<" / "<<nrPassFail->nrPass+nrPassFail->nrFail<<std::endl;
  }
};
  

HEEPV70Example::HEEPV70Example(const edm::ParameterSet& iPara,const NrPassFail*)
{
  //the sharp eyed amoungst you will notice I use the "vid" tag twice
  //this is because VID products have the same label (just different types)
  //the exception is for the bitmap as VID was already producing an unsigned int so it needed a different name
  eleAODToken_=consumes<edm::View<reco::GsfElectron> >(iPara.getParameter<edm::InputTag>("elesAOD"));
  eleMiniAODToken_=consumes<edm::View<reco::GsfElectron> >(iPara.getParameter<edm::InputTag>("elesMiniAOD"));
  vidPassToken_=consumes<edm::ValueMap<bool> >(iPara.getParameter<edm::InputTag>("vid"));
  vidBitmapToken_=consumes<edm::ValueMap<unsigned int> >(iPara.getParameter<edm::InputTag>("vidBitmap"));
  vidResultToken_=consumes<edm::ValueMap<vid::CutFlowResult> >(iPara.getParameter<edm::InputTag>("vid"));
  nrSatCrysMapToken_=consumes<edm::ValueMap<int> >(iPara.getParameter<edm::InputTag>("nrSatCrysMap"));
  trkIsoMapToken_=consumes<edm::ValueMap<float> >(iPara.getParameter<edm::InputTag>("trkIsoMap"));
  
}


void HEEPV70Example::analyze(const edm::Event& iEvent,const edm::EventSetup& iSetup)
{
  edm::Handle<edm::View<reco::GsfElectron> > eleHandle;

  edm::Handle<edm::ValueMap<bool> > vidPass;
  edm::Handle<edm::ValueMap<unsigned int> > vidBitmap;
  edm::Handle<edm::ValueMap<vid::CutFlowResult> > vidResult;
  
  edm::Handle<edm::ValueMap<int> > nrSatCrysMap;
  edm::Handle<edm::ValueMap<float> > trkIsoMap;

  //we done know if we have miniAOD or AOD, try both, AOD first and get
  //miniAOD if AOD not present
  iEvent.getByToken(eleAODToken_,eleHandle);
  if(!eleHandle.isValid()) iEvent.getByToken(eleMiniAODToken_,eleHandle);
  

  iEvent.getByToken(vidPassToken_,vidPass);
  iEvent.getByToken(vidBitmapToken_,vidBitmap);
  iEvent.getByToken(vidResultToken_,vidResult);
  iEvent.getByToken(trkIsoMapToken_,trkIsoMap);
  iEvent.getByToken(nrSatCrysMapToken_,nrSatCrysMap);

  for(size_t eleNr=0;eleNr<eleHandle->size();eleNr++){  
    edm::Ptr<reco::GsfElectron> elePtr(eleHandle,eleNr); //note we use an edm::Ptr rather than an edm::Ref
                                                         //as we do not know if its a pat::Electron 
                                                         //or a reco::GsfElectron
   
    //this allows to tell if the electron passed HEEPV70, true = passed
    bool passHEEPV70=(*vidPass)[elePtr]; 

    //lets count the number of pass / fail so we can compare against the reference
    if(passHEEPV70) globalCache()->nrPass++;
    else globalCache()->nrFail++;
    
    //this gives us to determine exactly which cuts the electron passed
    //each bit of this unsigned int corresponds to a cut, 0=fail, 1 =pass
    unsigned int heepV70Bitmap = (*vidBitmap)[elePtr];

    //to make it easier to use, a small class "cutnrs::HEEPV70" (in CutNrs.h")  been created which
    //has sensibly named enum values corresponding to the cut index
    //eg track isolation is cut number 7, so it defines a value TRKISO which evaluates to 7
    //additionally the templated class VidCutCodes provides functions
    //to determine if a set of cuts were passed or not
    //it inherits from its template arguement so cutnrs::HEEPV70::TRKISO and 
    //VIDCutCodes<cutnrs::HEEPV70 are identical
    
    using HEEPV70 = VIDCutCodes<cutnrs::HEEPV70>; 
    //note, here we use a std::intialisation list (the {...}) to make 
    //vector of all the cuts we would like to pass
    const bool passEtShowerShapeHE = HEEPV70::pass(heepV70Bitmap,{HEEPV70::ET,HEEPV70::SIGMAIETAIETA,HEEPV70::E2X5OVER5X5,HEEPV70::HADEM});
    //we can also tell it to ignore the cuts specified
    //eg lets require all cuts except tracker isolation
    const bool passN1TrkIso = HEEPV70::pass(heepV70Bitmap,HEEPV70::TRKISO,HEEPV70::IGNORE);

    //access # saturated crystals in the 5x5
    int nrSatCrys=(*nrSatCrysMap)[elePtr];
    if(nrSatCrys!=0) std::cout <<"nrSatCrys "<<nrSatCrys<<std::endl;
     
     //access new tracker isolation
    float trkIso=(*trkIsoMap)[elePtr];
    
    const vid::CutFlowResult& heepCutFlowResult = (*vidResult)[elePtr];

     //now we are going to access all of the information above via the vid::CutFlowResult 
    //well except for the nrSatCrys which vid doesnt store

    //how to check if everything passed:
    const bool passHEEPV70VID = heepCutFlowResult.cutFlowPassed();

    if(passHEEPV70!=passHEEPV70VID) std::cout <<"error in VID HEEP ID result "<<std::endl;
    
    //how to get the track isolation from VID
    //note, this works for all cuts except E2x5/E5x5 although this is a feature which is
    //not often used so safer to use the standard accessors
    //trk isolation value is confirmed to be okay though
    const float trkIsoVID = heepCutFlowResult.getValueCutUpon(HEEPV70::TRKISO);
    if(trkIso!=trkIsoVID) std::cout <<"error in VID trk iso "<<std::endl;
    
    //now lets do selective cuts like we did before
    const bool passEtShowerShapeHEVID = heepCutFlowResult.getCutResultByIndex(HEEPV70::ET)
      && heepCutFlowResult.getCutResultByIndex(HEEPV70::SIGMAIETAIETA) 
      && heepCutFlowResult.getCutResultByIndex(HEEPV70::E2X5OVER5X5)
      && heepCutFlowResult.getCutResultByIndex(HEEPV70::HADEM);

    //now for track isolation
    //now this may not be the fastest function as it makes a new cut flow...
    const bool passN1TrkIsoVID = heepCutFlowResult.getCutFlowResultMasking(HEEPV70::TRKISO).cutFlowPassed();

    //for debuging, all values here printed should be over 5 GeV
    if(passN1TrkIso && !passHEEPV70) std::cout <<" trk isol "<<trkIso<<std::endl;
    
    if(passEtShowerShapeHE != passEtShowerShapeHEVID) std::cout <<"error in VID showershape cuts"<<std::endl;
    if(passN1TrkIso != passN1TrkIsoVID) std::cout <<"error in VID trk iso cuts"<<std::endl;
   

  }
}

DEFINE_FWK_MODULE(HEEPV70Example);
