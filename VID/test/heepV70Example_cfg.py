import FWCore.ParameterSet.Config as cms

from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing ('analysis')
options.register ('useMiniAOD',
                  False,
                  VarParsing.multiplicity.singleton,
                  VarParsing.varType.bool,
                  "use miniAOD rather than AOD")
options.parseArguments()
useMiniAOD=options.useMiniAOD

# set up process
process = cms.Process("HEEP")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport = cms.untracked.PSet(
    reportEvery = cms.untracked.int32(1000),
    limit = cms.untracked.int32(10000000)
)
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')

#setup global tag
from Configuration.AlCa.GlobalTag import GlobalTag
from Configuration.AlCa.autoCond import autoCond
process.GlobalTag = GlobalTag(process.GlobalTag, '80X_mcRun2_asymptotic_2016_TrancheIV_v4', '') #


process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1) )
process.source = cms.Source ("PoolSource",fileNames = cms.untracked.vstring(
        '/store/mc/RunIISpring16DR80/ZToEE_NNPDF30_13TeV-powheg_M_200_400/RAWAODSIM/PUSpring16RAWAODSIM_80X_mcRun2_asymptotic_2016_v3-v1/50000/FAB476A3-4F13-E611-B591-008CFAF06402.root',
)
                                       
)

if useMiniAOD==True:
    process.source.fileNames=cms.untracked.vstring('/store/mc/RunIISpring16MiniAODv2/ZToEE_NNPDF30_13TeV-powheg_M_200_400/MINIAODSIM/PUSpring16RAWAODSIM_reHLT_80X_mcRun2_asymptotic_v14-v1/20000/0C242E56-BA3A-E611-9B2D-0242AC130004.root',)

#we now setup VID for HEEP V7.0, we will do this with the handy function below
#which creates the standard process.egmGsfElectronIDSequence for VID which we
#add to our path
from HEEP.VID.tools import setupVIDForHEEPV70
setupVIDForHEEPV70(process,useMiniAOD=useMiniAOD)

#this is our example analysis module reading the results, you will have your own module
process.heepIdExample = cms.EDAnalyzer("HEEPV70Example",
                                       elesAOD=cms.InputTag("gedGsfElectrons"),
                                       elesMiniAOD=cms.InputTag("slimmedElectrons"),
                                       nrSatCrysMap=cms.InputTag("heepIDVarValueMaps","eleNrSaturateIn5x5"),
                                       trkIsoMap=cms.InputTag("heepIDVarValueMaps","eleTrkPtIso"),
                                       vid=cms.InputTag("egmGsfElectronIDs:heepElectronID-HEEPV70"),
                                       vidBitmap=cms.InputTag("egmGsfElectronIDs:heepElectronID-HEEPV70Bitmap")
                                       )

process.p = cms.Path(
    process.egmGsfElectronIDSequence* 
    process.heepIdExample) #our analysing example module, replace with your module

#dumps the products made for easier debugging, you wouldnt normally need to do this
#edmDumpEventContent outputTest.root shows you all the products produced
#will be very slow when this is happening
process.load('Configuration.EventContent.EventContent_cff')
process.output = cms.OutputModule("PoolOutputModule",
    compressionAlgorithm = cms.untracked.string('LZMA'),
    compressionLevel = cms.untracked.int32(4),
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('MINIAODSIM'),
        filterName = cms.untracked.string('')
    ),
    dropMetaData = cms.untracked.string('ALL'),
    eventAutoFlushCompressedSize = cms.untracked.int32(15728640),
    fastCloning = cms.untracked.bool(False),
    fileName = cms.untracked.string('outputTest.root'),
    outputCommands = process.MINIAODSIMEventContent.outputCommands,
    overrideInputFileSplitLevels = cms.untracked.bool(True)
)
process.output.outputCommands = cms.untracked.vstring('keep *_*_*_*',
                                                           )
process.outPath = cms.EndPath(process.output)
