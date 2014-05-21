//
//  optional.h
//  ngn
//
//  Created by Christopher Tarquini on 1/24/14.
//
//

#ifndef ngn_optional_h
#define ngn_optional_h

#ifndef NGN_USE_BOOST
#ifdef NGN_USE_OPTIONAL_STANDALONE
#include "optional-standalone.h"
#endif
#else 
namespace ngn {
    typedef boost::optional optional;
}
#endif

#endif
