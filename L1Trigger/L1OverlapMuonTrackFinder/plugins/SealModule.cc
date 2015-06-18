#include "FWCore/PluginManager/interface/ModuleDef.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "OMTFProducerMix.h"
DEFINE_FWK_MODULE(OMTFProducerMix);

#include "OMTFProducer.h"
DEFINE_FWK_MODULE(OMTFProducer);

#include "OMTFPatternMaker.h"
DEFINE_FWK_MODULE(OMTFPatternMaker);

