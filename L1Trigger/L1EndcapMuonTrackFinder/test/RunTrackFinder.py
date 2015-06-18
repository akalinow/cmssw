import FWCore.ParameterSet.Config as cms

process = cms.Process('TEXTDUMP')
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 1
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(1))
#process.SimpleMemoryCheck = cms.Service("SimpleMemoryCheck",ignoreTotal = cms.untracked.int32(1))
process.load("L1TriggerConfig.L1ScalesProducers.L1MuTriggerScalesConfig_cff")
process.load("L1TriggerConfig.L1ScalesProducers.L1MuTriggerPtScaleConfig_cff")
process.load("Configuration.StandardSequences.Geometry_cff")



process.options = cms.untracked.PSet(
    SkipEvent = cms.untracked.vstring('ProductNotFound')
)

process.L1TMuonTrkFinder = cms.EDProducer(
    'L1TMuonUpgradedTrackFinder',
    doGen = cms.untracked.bool(True),
    genSrc = cms.untracked.InputTag("genParticles"),
    primitiveSrcs = cms.VInputTag(
    cms.InputTag('L1TMuonTriggerPrimitives','CSC'),
    cms.InputTag('L1TMuonTriggerPrimitives','DT'),
    cms.InputTag('L1TMuonTriggerPrimitives','RPC')
    ),
    converterSrcs = cms.VInputTag(
    cms.InputTag('L1CSCTFTrackConverter'),
    cms.InputTag('L1DTTFTrackConverter'),
    cms.InputTag('L1RPCbTFTrackConverter'),
    cms.InputTag('L1RPCfTFTrackConverter'),
    cms.InputTag('L1TMuonSimpleDeltaEtaHitMatcher')
    ),
    lutParam = cms.PSet(
    isBeamStartConf = cms.untracked.bool(True),
    ReadPtLUT = cms.bool(False),
    PtMethod = cms.untracked.uint32(32)
    )
)

process.content = cms.EDAnalyzer("EventContentAnalyzer")

infile = 'file:/afs/cern.ch/work/m/mcarver/MuonDevelopment/CMSSW_7_2_0_pre6/src/L1Trigger/L1TMuon/test/Sector1Events/1p2_2p0_Job1.root'
fileOutName = "Histos.root"

process.source = cms.Source(
    'PoolSource',
    fileNames = cms.untracked.vstring(infile)
    
    )

process.TFileService = cms.Service("TFileService",
    fileName = cms.string(
	fileOutName
))


outCommands = cms.untracked.vstring('keep *')

process.FEVTDEBUGoutput = cms.OutputModule(
    "PoolOutputModule",
    splitLevel = cms.untracked.int32(0),
    eventAutoFlushCompressedSize = cms.untracked.int32(5242880),
    outputCommands = outCommands,
    fileName = cms.untracked.string('Emulator_Out.root'),
    dataset = cms.untracked.PSet(
        filterName = cms.untracked.string(''),
        dataTier = cms.untracked.string('')
    )
)

process.L1TMuonSequence = cms.Sequence(process.L1TMuonTrkFinder)

process.L1TMuonPath = cms.Path(process.L1TMuonSequence)

process.outPath = cms.EndPath(process.FEVTDEBUGoutput)

process.schedule = cms.Schedule(process.L1TMuonPath,process.outPath)

