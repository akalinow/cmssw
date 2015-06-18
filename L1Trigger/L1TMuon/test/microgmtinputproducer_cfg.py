import FWCore.ParameterSet.Config as cms

process = cms.Process("L1MicroGMTInputs")

process.load("FWCore.MessageService.MessageLogger_cfi")

# max events has to match what is in the .dat file:
# single_muons: 108, single_tfs: 6, many_events: 91, sort_test: 7
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(91))

process.load("L1Trigger.L1TMuon.microgmtinputproducer_cfi")

process.uGMTInputProducer.inputFileName = "patterns/many_events.dat"

process.source = cms.Source("EmptySource",
)
process.out = cms.OutputModule("PoolOutputModule",
    fileName = cms.untracked.string('ugmt_input_many_events.root')
)
  
process.p = cms.Path(process.uGMTInputProducer)

process.e = cms.EndPath(process.out)

