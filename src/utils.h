//
//  utils.h
//  uv
//
//  Created by Christopher Tarquini on 1/15/14.
//  Copyright (c) 2014 Christopher Tarquini. All rights reserved.
//

#ifndef __uv__utils__
#define __uv__utils__
#include <stddef.h>

namespace ngn {
    namespace utils {
        template<class Type, typename MemberType>
        constexpr inline ptrdiff_t offset_of(MemberType Type::* Member){
            return reinterpret_cast<ptrdiff_t>(reinterpret_cast<const char*>(&(reinterpret_cast<Type*>(8)->*Member)) - 8);
        };
        
        template<class Type, typename MemberType>
        Type* container_of(MemberType* ptr, MemberType Type::* Member){
            return reinterpret_cast<Type*>(reinterpret_cast<char*>(ptr) - offset_of<Type, MemberType>(Member));
        };
        
        unsigned long next_power_of_2(unsigned long v);
    }
}

#endif /* defined(__uv__utils__) */
