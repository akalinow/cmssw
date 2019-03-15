# -*- coding: utf-8 -*-
import FWCore.ParameterSet.Config as cms
process = cms.Process("L1TMuonEmulation")
import os
import sys
import commands

process.load("FWCore.MessageLogger.MessageLogger_cfi")

process.MessageLogger.cerr.FwkReport.reportEvery = cms.untracked.int32(5000)
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(True))

path = "file:/home/akalinow/scratch/CMS/OverlapTrackFinder/Data/SingleMu/9_3_14_FullEta_v2/"
process.source = cms.Source('PoolSource',
                            fileNames = cms.untracked.vstring(path+"SingleMu_20_p_10.root"),
                            inputCommands=cms.untracked.vstring(
                                'keep *',
                                'drop l1tEMTFHit2016Extras_simEmtfDigis_CSC_HLT',
                                'drop l1tEMTFHit2016Extras_simEmtfDigis_RPC_HLT',
                                'drop l1tEMTFHit2016s_simEmtfDigis__HLT',
                                'drop l1tEMTFTrack2016Extras_simEmtfDigis__HLT',
                                'drop l1tEMTFTrack2016s_simEmtfDigis__HLT')
)
'''
path = "file:/home/akalinow/scratch/CMS/OverlapTrackFinder/Data/SingleMu/9_3_14_FullEta_v2/"
command = "ls "+dataPath+"/*/*/*/*_{1,2,3,4,5}.root"
fileList = commands.getoutput(command).split("\n")   
process.source.fileNames =  cms.untracked.vstring()
for aFile in fileList:
    process.source.fileNames.append('file:'+aFile)
'''
	                    
process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(-1))

# 2023 geometry
process.load('Configuration.Geometry.GeometryExtended2023D17Reco_cff')
process.load('Configuration.Geometry.GeometryExtended2023D17_cff')
############################
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
from Configuration.AlCa.GlobalTag_condDBv2 import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:upgradePLS3', '')


####Event Setup Producer
process.load('L1Trigger.L1TMuonOverlap.fakeOmtfParams_cff')
process.omtfParams.configXMLFile =  cms.FileInPath("L1Trigger/L1TMuonOverlap/test/hwToLogicLayer_bmtf_v1.xml")
process.omtfParams.patternsXMLFiles = cms.VPSet(cms.PSet(patternsXMLFile = cms.FileInPath("L1Trigger/L1TMuonOverlap/test/GPs_bmtf_v1.xml")))

process.esProd = cms.EDAnalyzer("EventSetupRecordDataGetter",
   toGet = cms.VPSet(
      cms.PSet(record = cms.string('L1TMuonOverlapParamsRcd'),
               data = cms.vstring('L1TMuonOverlapParams'))
                   ),
   verbose = cms.untracked.bool(False)
)

####OMTF Emulator
process.load('L1Trigger.L1TMuonOverlap.simOmtfDigis_cfi')

####OMTF Analyzer
process.load('omtfTree_cfi')

###Gen level filter configuration
process.MuonEtaFilter = cms.EDFilter("SimTrackEtaFilter",
                                minNumber = cms.uint32(1),
                                src = cms.InputTag("g4SimHits"),
                                #cut = cms.string("momentum.eta<1.24 && momentum.eta>0.83 &&  momentum.pt>1")
                                cut = cms.string("momentum.pt>1")
                                )

process.L1TMuonSeq = cms.Sequence(process.esProd + process.simOmtfDigis + process.omtfTree)
process.GenMuPath = cms.Path(process.MuonEtaFilter)
process.L1TMuonPath = cms.Path(process.MuonEtaFilter*process.L1TMuonSeq)
process.schedule = cms.Schedule(process.GenMuPath,process.L1TMuonPath)

