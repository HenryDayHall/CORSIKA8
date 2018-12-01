#ifndef _include_VOLUME_H_
#define _include_VOLUME_H_

#include <corsika/geometry/Point.h>

namespace corsika::geometry {

  class Volume {
  
  public:
    //! returns true if the Point p is within the volume
    virtual bool Contains(Point const& p) const = 0;
    
    virtual ~Volume() = default;
  };

} // namespace corsika::geometry

#endif
