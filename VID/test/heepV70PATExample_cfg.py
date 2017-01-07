import FWCore.ParameterSet.Config as cms

from FWCore.ParameterSet.VarParsing import VarParsing

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
        '/store/mc/RunIISpring16MiniAODv2/ZToEE_NNPDF30_13TeV-powheg_M_200_400/MINIAODSIM/PUSpring16RAWAODSIM_reHLT_80X_mcRun2_asymptotic_v14-v1/20000/0C242E56-BA3A-E611-9B2D-0242AC130004.root',
        )
)

from HEEP.VID.tools import addHEEPV70ElesMiniAOD
addHEEPV70ElesMiniAOD(process,useStdName=True)

#this is our example analysis module reading the results
process.heepIdExample = cms.EDAnalyzer("HEEPV70PATExample",
                                       eles=cms.InputTag("slimmedElectrons"),
                                       )

process.p = cms.Path(
    process.heepSequence*
    process.heepIdExample) #our analysing example module, replace with your module

#dumps the products made for easier debugging
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
