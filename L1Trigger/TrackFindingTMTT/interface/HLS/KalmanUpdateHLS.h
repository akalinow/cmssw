///=== This is the KF updator code used in HLS firmware.
///=== N.B. It therefore can't use the Settings class or any external libraries! Nor can it be a C++ class.
 
#ifndef __KalmanUpdateHLS__
#define __KalmanUpdateHLS__

// Defines StateHLS & KFstateHLS. Also defines finite bit integers & floats.
#ifdef CMSSW_GIT_HASH
#include "L1Trigger/TrackFindingTMTT/interface/HLS/KF4interfaceHLS.h"
#include "L1Trigger/TrackFindingTMTT/interface/HLS/KalmanMatricesHLS.h"
#include "L1Trigger/TrackFindingTMTT/interface/HLS/HLSutilities.h"
#else
#include "KF4interfaceHLS.h"
#include "KalmanMatricesHLS.h"
#include "HLSutilities.h"
#endif
 
#ifdef CMSSW_GIT_HASH
namespace TMTT {

namespace KalmanHLS {
#endif

// Add stub to old KF helix state to get new KF helix state.
void KalmanUpdateHLS(const StubHLS& stub, const KFstateHLS& stateIn, KFstateHLS& stateOut, ExtraOutHLS& extraOut);

// Calculate increase in chi2 from adding new stub: delta(chi2) = res(transpose) * R(inverse) * res
TCHI_INT calcDeltaChi2(const VectorRes& res, const MatrixInverseR& Rinv);

#ifdef CMSSW_GIT_HASH
}

}
#endif

#endif




