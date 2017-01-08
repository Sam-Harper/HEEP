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

namespace heepV70 {
  enum CutIndex {
    ET=0,ETA,DETAINSEED,DPHIIN,SIGMAIETAIETA,E2X5OVER5X5,HADEM,TRKISO,EMHADD1ISO,DXY,MISSHITS,ECALDRIVEN
  };
}


class HEEPV70PATValidation : public edm::stream::EDAnalyzer<> {

private:
 
  edm::EDGetTokenT<edm::View<pat::Electron> > elesToken_;
  edm::EDGetTokenT<edm::View<pat::Electron> > orgElesToken_;
  edm::EDGetTokenT<edm::ValueMap<bool> > vidToken_; //VID is versioned ID, is the standard E/gamma ID producer which we have configured for HEEP
  edm::EDGetTokenT<edm::ValueMap<float> > trkIsoMapToken_;
  edm::EDGetTokenT<edm::ValueMap<int> > nrSatCrysMapToken_;

  
public:
  explicit HEEPV70PATValidation(const edm::ParameterSet& iPara);
  virtual ~HEEPV70PATValidation(){}
  
private:
  void analyze(const edm::Event& iEvent,const edm::EventSetup& iSetup) override;
};


template <typename T>
bool hasSameOrdering(const T& coll1,const T& coll2){
  if(coll1.size()!=coll2.size()) return false;
  
  for(size_t objNr=0;objNr<coll1.size();objNr++){
    auto& obj1 = coll1[objNr];
    auto& obj2 = coll2[objNr];
      
    if(reco::deltaR2(obj1.eta(),obj1.phi(),obj2.eta(),obj2.phi())>0.001) return false;
  }
  return true;
}

void throwCollectionInvalidException( edm::Handle<edm::View<pat::Electron> > elesHandle,
				      edm::Handle<edm::View<pat::Electron> > orgElesHandle)
{
  std::ostringstream errMsg;
  errMsg<< "ordering of orginal and new electron collections are different:";
  size_t nrEles = elesHandle->size();
  size_t nrElesOrg = orgElesHandle->size();
  size_t maxNrEles = std::max(nrEles,nrElesOrg);
  auto printEle=[](const auto& ele,std::ostream& out){out <<"E_{T} "<<ele.et()<<" phi "<<ele.phi()<<" eta "<<ele.eta();};
  for(size_t eleNr=0;eleNr<maxNrEles;eleNr++){
    errMsg<<"  ele "<<eleNr<<" org ";
    if(eleNr<nrElesOrg) printEle((*elesHandle)[eleNr],errMsg);
    else errMsg<<" N/A";
    errMsg<<" new ";
    if(eleNr<nrElesOrg) printEle((*orgElesHandle)[eleNr],errMsg);
    else errMsg<<" N/A";
      errMsg<<std::endl;
  }
  throw cms::Exception("CollectionOrderInvalid",errMsg.str());
}

  

HEEPV70PATValidation::HEEPV70PATValidation(const edm::ParameterSet& iPara)
{
  elesToken_=consumes<edm::View<pat::Electron> >(iPara.getParameter<edm::InputTag>("eles"));
  orgElesToken_=consumes<edm::View<pat::Electron> >(iPara.getParameter<edm::InputTag>("orgEles"));
  vidToken_=consumes<edm::ValueMap<bool> >(iPara.getParameter<edm::InputTag>("vid"));
  trkIsoMapToken_=consumes<edm::ValueMap<float> >(iPara.getParameter<edm::InputTag>("trkIsoMap"));
  nrSatCrysMapToken_=consumes<edm::ValueMap<int> >(iPara.getParameter<edm::InputTag>("nrSatCrysMap"));
}


void HEEPV70PATValidation::analyze(const edm::Event& iEvent,const edm::EventSetup& iSetup)
{
  edm::Handle<edm::View<pat::Electron> > elesHandle;
  edm::Handle<edm::View<pat::Electron> > orgElesHandle;
  edm::Handle<edm::ValueMap<bool> > vidHandle;
  edm::Handle<edm::ValueMap<float> > trkIsoMapHandle;
  edm::Handle<edm::ValueMap<int> > nrSatCrysMapHandle;
  
  iEvent.getByToken(elesToken_,elesHandle);
  iEvent.getByToken(orgElesToken_,orgElesHandle);
  iEvent.getByToken(vidToken_,vidHandle);
  iEvent.getByToken(trkIsoMapToken_,trkIsoMapHandle);
  iEvent.getByToken(nrSatCrysMapToken_,nrSatCrysMapHandle);

  if(!hasSameOrdering(*elesHandle,*orgElesHandle)){
    //throws an exception indicating an error
    throwCollectionInvalidException(elesHandle,orgElesHandle);
  }
   
  
  for(size_t eleNr=0;eleNr<elesHandle->size();eleNr++){  
    edm::Ptr<pat::Electron> orgElePtr(orgElesHandle,eleNr);
    edm::Ptr<pat::Electron> elePtr(elesHandle,eleNr);
    
    const bool passHEEPOrg=(*vidHandle)[orgElePtr]; 
    const float trkIsoOrg=(*trkIsoMapHandle)[orgElePtr];
    const float nrSatCrysOrg=(*nrSatCrysMapHandle)[orgElePtr];
    
    const vid::CutFlowResult* vidResult =  elePtr->userData<vid::CutFlowResult>("heepElectronID_HEEPV70");
    
    const bool passHEEPUserInt = elePtr->userInt("heepElectronID_HEEPV70");
    const bool passHEEPVID = vidResult->cutFlowPassed();
    const float trkIso = elePtr->userFloat("trkPtIso");
    const float trkIsoVID = vidResult->getValueCutUpon(heepV70::TRKISO);
    const float nrSatCrys = elePtr->userInt("nrSatCrys");

    bool failValid = false;
    if(passHEEPUserInt!=passHEEPOrg || passHEEPUserInt!=passHEEPVID) failValid=true;
    if(trkIsoOrg!=trkIso || trkIso!=trkIsoVID) failValid=true;
    if(nrSatCrysOrg!=nrSatCrys) failValid=true;
    
    if(failValid){
      std::cout <<"for event "<<iEvent.id().run()<<" "<<iEvent.luminosityBlock()<<" "<<iEvent.id().event()<<" ele "<<eleNr<<" failed validation"<<std::endl;
      std::cout <<"  et "<<elePtr->et()<<" eta "<<elePtr->eta()<<" phi "<<elePtr->phi()<<std::endl;
      std::cout <<"  heepID : org "<<passHEEPOrg<<" UserInt "<<passHEEPUserInt<<" VID "<<passHEEPVID<<std::endl;
      std::cout <<"  trkIso : org "<<trkIsoOrg<<" UserFloat "<<trkIso<<" VID "<<trkIsoVID<<" CMSSW  value "<<elePtr->dr03TkSumPt()<<std::endl;
      std::cout <<"  nrSatCrys : org "<<nrSatCrysOrg<<" UserInt "<<elePtr->userInt("nrSatCrys")<<std::endl;
    }
    
  }
}

DEFINE_FWK_MODULE(HEEPV70PATValidation);
 
