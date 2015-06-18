#include "../interface/MicroGMTConfiguration.h"

unsigned 
l1t::MicroGMTConfiguration::getTwosComp(const int signed_int, const int width) {
  if (signed_int >= 0) {
    return (unsigned)signed_int;
  }
  int all_one = (1 << width)-1;
  return ((-signed_int) ^ all_one) + 1;
}
