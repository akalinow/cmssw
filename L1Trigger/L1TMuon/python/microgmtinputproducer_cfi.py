import FWCore.ParameterSet.Config as cms

uGMTInputProducer = cms.EDProducer('l1t::uGMTInputProducer',
    inputFileName = cms.string("../test/test.dat")
)
