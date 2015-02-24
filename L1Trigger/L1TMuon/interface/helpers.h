#ifndef __L1TMUON_HELPERS_H__
#define __L1TMUON_HELPERS_H__
// 
// Info: This is a collection of helpful free functions to making dealing
//       trigger primitives a bit more straight forward.
//
// Author: L. Gray (FNAL)
//

#include "DataFormats/L1TMuon/interface/L1TMuonTriggerPrimitiveFwd.h"
#include "DataFormats/L1TMuon/interface/L1TMuonTriggerPrimitive.h"
#include "DataFormats/Common/interface/Handle.h"

namespace L1TMuon {
  namespace helpers {
    TriggerPrimitiveList
      getPrimitivesByCSCTriggerInfo(const int endcap,
				    const int sector,
			    const edm::Handle<TriggerPrimitiveCollection>& tps,
				    const std::vector<unsigned>& trkNmbs);
    TriggerPrimitiveList 
      getPrimitivesByDTTriggerInfo(const int wheel,
				   const int sp_wheel,
				   const int sector,
			    const edm::Handle<TriggerPrimitiveCollection>& tps,
				   const unsigned mode,
				   const std::vector<unsigned>& addrs);
  }
}

#endif
