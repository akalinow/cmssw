import FWCore.ParameterSet.Config as cms
process = cms.Process("L1TMuonEmulation")
import os
import sys
import commands

process.load("FWCore.MessageLogger.MessageLogger_cfi")

verbose = False

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
    process.MessageLogger.cerr.FwkReport.reportEvery = cms.untracked.int32(5000)

process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(True))

process.source = cms.Source(
    'PoolSource',
    fileNames = cms.untracked.vstring('file:/afs/cern.ch/work/j/jlingema/public/omtf_input_test.root')
    )

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(500))

###PostLS1 geometry used
process.load('Configuration.Geometry.GeometryExtendedPostLS1Reco_cff')
process.load('Configuration.Geometry.GeometryExtendedPostLS1_cff')
############################
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
from Configuration.AlCa.GlobalTag_condDBv2 import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_mc', '')

path = os.environ['CMSSW_BASE']+"/src/L1Trigger/L1OverlapMuonTrackFinder/data/"

process.load('L1Trigger.L1EndcapMuonTrackFinder.L1TMuonTriggerPrimitiveProducer_cfi')

###OMTF emulator configuration
process.omtfEmulator = cms.EDProducer("OMTFProducer",
                                      TriggerPrimitiveSrc = cms.InputTag('L1TMuonTriggerPrimitives'),
                                      dumpResultToXML = cms.bool(False),
                                      XMLDumpFileName = cms.string("TestEvents.xml"),                                     
                                      dumpGPToXML = cms.bool(False),  
                                      readEventsFromXML = cms.bool(False),
                                      eventsXMLFiles = cms.vstring("TestEvents.xml"),
                                      dropRPCPrimitives = cms.bool(False),                                    
                                      dropDTPrimitives = cms.bool(False),                                    
                                      dropCSCPrimitives = cms.bool(False),   
                                      omtf = cms.PSet(
        configXMLFile = cms.string(path+"hwToLogicLayer_721_5760.xml"),
        patternsXMLFiles = cms.vstring(path+"Patterns_ipt4_31_5760.xml"),
        )
                                      )

process.uGMTInputProducer = cms.EDProducer("l1t::uGMTInputProducerFromGen",
)

process.load("L1Trigger.L1TMuon.microgmtemulator_cfi")

process.microGMTEmulator.overlapTFInput = cms.InputTag("omtfEmulator", "OMTF")

process.L1TMuonSeq = cms.Sequence( process.L1TMuonTriggerPrimitives +
                                   # process.emtfEmulator +
                                   process.omtfEmulator +
                                   process.uGMTInputProducer +
                                   process.microGMTEmulator)
                                   #  +
                                   # process.emtfEmulator + 
                                   # process.bmtfEmulator +
                                   # process.caloEmulator)

process.L1TMuonPath = cms.Path(process.L1TMuonSeq)


process.out = cms.OutputModule("PoolOutputModule", 
   fileName = cms.untracked.string("l1tmuon_test.root"),
                               )

process.output_step = cms.EndPath(process.out)

process.schedule = cms.Schedule(process.L1TMuonPath)
process.schedule.extend([process.output_step])
