#ifndef HEEP_VID_CutNrs_h
#define HEEP_VID_CutNrs_h

namespace cutnrs {
  class HEEPV70 {
  public:
    enum CutIndex {
      ET=0,ETA=1,DETAINSEED=2,DPHIIN=3,SIGMAIETAIETA=4,E2X5OVER5X5=5,HADEM=6,TRKISO=7,EMHADD1ISO=8,DXY=9,MISSHITS=10,ECALDRIVEN=11
    };
    
    constexpr static unsigned int kMaxBitNr=ECALDRIVEN;
    constexpr static unsigned int kFullMask=( 0x1 << (kMaxBitNr+1) ) -1;
  };
  class CutSum16V1 {
  public:
    enum CutIndex {
      ET=0,ETA=1,DETAINSEED=2,DPHIIN=3,SIGMAIETAIETA=4,HADEM=5,INVEINVP=6,PFISO=7,CONVETO=8,MISSHITS=9
    };
    constexpr static unsigned int kMaxBitNr=MISSHITS;
    constexpr static unsigned int kFullMask=( 0x1 << (kMaxBitNr+1) ) -1;
  };
  
  class CutFall17V1 {
  public:
    enum CutIndex {
      ET=0,ETA=1,DETAINSEED=2,DPHIIN=3,SIGMAIETAIETA=4,HADEM=5,INVEINVP=6,PFISO=7,CONVETO=8,MISSHITS=9
    };
    constexpr static unsigned int kMaxBitNr=MISSHITS;
    constexpr static unsigned int kFullMask=( 0x1 << (kMaxBitNr+1) ) -1;
  };
  

}

#endif
