import FWCore.ParameterSet.Config as cms

def addHEEPV70ElesMiniAOD(process,useStdName=True):
    #setup the VID with HEEP 7.0
    from PhysicsTools.SelectorUtils.tools.vid_id_tools import switchOnVIDElectronIdProducer
    from PhysicsTools.SelectorUtils.tools.vid_id_tools import setupAllVIDIdsInModule
    # turn on VID producer, indicate data format  to be
    # DataFormat.AOD or DataFormat.MiniAOD, as appropriate
    switchOnVIDElectronIdProducer(process, DataFormat.MiniAOD)

    # define which IDs we want to produce
    my_id_modules = ['RecoEgamma.ElectronIdentification.Identification.heepElectronID_HEEPV70_cff']
                   #add them to the VID producer
    for idmod in my_id_modules:
        setupAllVIDIdsInModule(process,idmod,setupVIDElectronSelection)

    process.heepIDVarValueMaps.elesMiniAOD = \
        cms.InputTag("slimmedElectrons",processName=cms.InputTag.skipCurrentProcess())
    process.egmGsfElectronIDs.physicsObjectSrc = \
        cms.InputTag("slimmedElectrons",processName=cms.InputTag.skipCurrentProcess())
        
    
    process.load("HEEP.VID.addHEEPV70ToEles_cfi") 
    process.heepSequence = cms.Sequence(process.egmGsfElectronIDSequence)
    if useStdName:
        process.heepSequence.insert(1,process.addHEEPToSlimmedElectrons)
    else:
        process.heepSequence.insert(1,process.addHEEPToHEEPElectrons)
        
                                      
    
