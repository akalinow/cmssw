# -*- coding: utf-8 -*-
import FWCore.ParameterSet.Config as cms
process = cms.Process("L1TMuonEmulation")
import os
import sys
import commands

process.load("FWCore.MessageLogger.MessageLogger_cfi")

process.MessageLogger.cerr.FwkReport.reportEvery = cms.untracked.int32(50)
process.options = cms.untracked.PSet(wantSummary = cms.untracked.bool(False))

process.source = cms.Source('PoolSource',
#				fileNames = cms.untracked.vstring('file:/afs/cern.ch/work/g/gflouris/public/SingleMuPt6180_noanti_50k_eta08.root')
 fileNames = cms.untracked.vstring('file:/afs/cern.ch/work/g/gflouris/public/SingleMuPt6180_noanti_10k_eta1.root')
	                    )

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(500))

# PostLS1 geometry used
process.load('Configuration.Geometry.GeometryExtended2015Reco_cff')
process.load('Configuration.Geometry.GeometryExtended2015_cff')
############################
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_condDBv2_cff')
from Configuration.AlCa.GlobalTag_condDBv2 import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_mc', '')


####Event Setup Producer
process.load('L1Trigger.L1TMuonTrackFinderBarrel.l1tmbtfparamsproducer_cfi')
process.esProd = cms.EDAnalyzer("EventSetupRecordDataGetter",
   toGet = cms.VPSet(
      cms.PSet(record = cms.string('L1TMuonBarrelParamsRcd'),
               data = cms.vstring('L1TMuonBarrelParams'))
                   ),
   verbose = cms.untracked.bool(True)
)


####BMTF Emulator
process.load('L1Trigger.L1TMuonBarrel.bmtfDigis_cfi')
process.bmtfDigis.DTDigi_Source = cms.InputTag("L1TTwinMuxProducer")
process.bmtfDigis.DTDigi_Theta_Source = cms.InputTag("simDtTriggerPrimitiveDigis")
process.bmtfDigis.Debug = cms.untracked.int32(0)


####TwinMux Emulator
process.load('L1Trigger.L1TMuonBarrel.L1TTwinMuxProducer_cfi')


process.L1TMuonSeq = cms.Sequence( process.esProd             +
				   process.L1TTwinMuxProducer +
                                   process.bmtfDigis 
)

process.L1TMuonPath = cms.Path(process.L1TMuonSeq)

process.out = cms.OutputModule("PoolOutputModule", 
   fileName = cms.untracked.string("l1tbmtf_superprimitives1.root"),
                               )

process.output_step = cms.EndPath(process.out)
process.schedule = cms.Schedule(process.L1TMuonPath)
process.schedule.extend([process.output_step])