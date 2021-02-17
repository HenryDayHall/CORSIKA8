#include <epos.hpp>

namespace epos {

  // this is needed as linker object, but it is not needed to do anything
  void ranfini_(double&, int&, int&) {}

  // this is needed as linker object, but it is not needed to do anything
  void ranfcv_(double&) {}

  double rangen_() { return  ::epos::rndm_interface(); }

}
	
