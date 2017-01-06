import FWCore.ParameterSet.Config as cms

from PhysicsTools.PatAlgos.slimming.modifiedElectrons_cfi import modifiedElectrons
from HEEP.VID.heepV70Modifier_cfi import heep_modifications

heepElectrons = modifiedElectrons.clone()
heepElectrons.modifierConfig.modifications = heep_modifications

slimmedElectrons = heepElectrons.clone()

addHEEPToSlimmedElectrons = cms.Sequence( slimmedElectrons )
addHEEPToHEEPElectrons = cms.Sequence( heepElectrons )

