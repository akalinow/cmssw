import FWCore.ParameterSet.Config as cms
process = cms.Process("OMTFEmulation")
import os
import sys
import commands

verbose = False

process.load("FWCore.MessageLogger.MessageLogger_cfi")

if verbose: 
    process.MessageLogger = cms.Service("MessageLogger",
       suppressInfo       = cms.untracked.vstring('AfterSource', 'PostModule'),
       destinations   = cms.untracked.vstring(
                                             'detailedInfo'
                                             ,'critical'
                                             ,'cout'
                    ),
       categories = cms.untracked.vstring(
                                        'CondDBESSource'
                                        ,'EventSetupDependency'
                                        ,'Geometry'
                                        ,'MuonGeom'
                                        ,'GetManyWithoutRegistration'
                                        ,'GetByLabelWithoutRegistration'
                                        ,'Alignment'
                                        ,'SiStripBackPlaneCorrectionDepESProducer'
                                        ,'SiStripLorentzAngleDepESProducer'
                                        ,'SiStripQualityESProducer'
                                        ,'TRACKER'
                                        ,'HCAL'
        ),
       critical       = cms.untracked.PSet(
                        threshold = cms.untracked.string('ERROR') 
        ),
       detailedInfo   = cms.untracked.PSet(
                      threshold  = cms.untracked.string('INFO'), 
                      CondDBESSource  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                      EventSetupDependency  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                      Geometry  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                      MuonGeom  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                      Alignment  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                      GetManyWithoutRegistration  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                      GetByLabelWithoutRegistration  = cms.untracked.PSet (limit = cms.untracked.int32(0) ) 

       ),
       cout   = cms.untracked.PSet(
                threshold  = cms.untracked.string('INFO'), 
                CondDBESSource  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                EventSetupDependency  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                Geometry  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                MuonGeom  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                Alignment  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                GetManyWithoutRegistration  = cms.untracked.PSet (limit = cms.untracked.int32(0) ), 
                GetByLabelWithoutRegistration  = cms.untracked.PSet (limit = cms.untracked.int32(0) ) 
                ),
                                        )

if not verbose:
    process.MessageLogger.cerr.FwkReport.reportEvery = cms.untracked.int32(50000)
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(True))

process.source = cms.Source(
    'PoolSource',
    fileNames = cms.untracked.vstring('file:/home/akalinow/scratch/CMS/OverlapTrackFinder/Crab/SingleMuFullEtaTestSample/720_FullEta_v1/data/SingleMu_16_p_1_2_TWz.root')
)

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1))

###PostLS1 geometry used
process.load('Configuration.Geometry.GeometryExtendedPostLS1Reco_cff')
process.load('Configuration.Geometry.GeometryExtendedPostLS1_cff')
############################
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
from Configuration.AlCa.GlobalTag_condDBv2 import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_mc', '')

path = os.environ['CMSSW_BASE']+"/src/L1Trigger/L1TMuon/data/"

process.load('L1Trigger.L1TMuon.L1TMuonTriggerPrimitiveProducer_cfi')

###OMTF pattern maker configuration
process.omtfPatternMaker = cms.EDAnalyzer("OMTFPatternMaker",
                                      TriggerPrimitiveSrc = cms.InputTag('L1TMuonTriggerPrimitives'),
                                      g4SimTrackSrc = cms.InputTag('g4SimHits'),
                                      makeGoldenPatterns = cms.bool(True),                                     
                                      makeConnectionsMaps = cms.bool(False),                                      
                                      dropRPCPrimitives = cms.bool(False),                                    
                                      dropDTPrimitives = cms.bool(False),                                    
                                      dropCSCPrimitives = cms.bool(False),   
                                      ptCode = cms.int32(16),
                                      charge = cms.int32(1),
                                      omtf = cms.PSet(
        configXMLFile = cms.string(path+"hwToLogicLayer_721_5760.xml"),
        patternsXMLFiles = cms.vstring(path+"Patterns_ipt4_31_5760.xml"),
        )
                                      )

process.L1TMuonSeq = cms.Sequence( process.L1TMuonTriggerPrimitives+ 
                                   process.omtfPatternMaker)

process.L1TMuonPath = cms.Path(process.L1TMuonSeq)

process.schedule = cms.Schedule(process.L1TMuonPath)

