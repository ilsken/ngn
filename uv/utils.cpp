//
//  utils.cpp
//  uv
//
//  Created by Christopher Tarquini on 1/15/14.
//  Copyright (c) 2014 Christopher Tarquini. All rights reserved.
//

#include "utils.h"

namespace ngn {
    namespace utils {
        unsigned long next_power_of_2(unsigned long v)
        {
            v--;
            v |= v >> 1;
            v |= v >> 2;
            v |= v >> 4;
            v |= v >> 8;
            v |= v >> 16;
            v |= v >> 32;
            v++;
            return v;
            
        }
    }
}

