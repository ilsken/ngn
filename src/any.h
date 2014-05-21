//
//  any.h
//  ngn
//
//  Created by Christopher Tarquini on 1/25/14.
//
//

#ifndef ngn_any_h
#define ngn_any_h

#ifndef NGN_USE_BOOST
#include "any-standalone.h"
#endif
#else
#include <boost/any.hpp>
namespace ngn {
    typedef boost::any any;
}
#endif


#endif
