//
//  internals.h
//  uv
//
//  Created by Christopher Tarquini on 10/15/13.
//  Copyright (c) 2013 Christopher Tarquini. All rights reserved.
//

#ifndef uv_internals_h
#define uv_internals_h


static inline void Swizzle(char* start, unsigned int len) {
    char* end = start + len - 1;
    while (start < end) {
        char tmp = *start;
        *start++ = *end;
        *end-- = tmp;
    }
}

#endif
