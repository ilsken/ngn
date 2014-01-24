//
//  wrapper.h
//  uv
//
//  Created by Christopher Tarquini on 10/11/13.
//  Copyright (c) 2013 Christopher Tarquini. All rights reserved.
//

#ifndef __uv__wrapper__
#define __uv__wrapper__

#include <iostream>
template <class UvType>
struct UvHandle{
    std::shared_ptr<char> handle;
    UvType item;
};


template<class Type, typename MemberType>
constexpr inline ptrdiff_t offset_of(MemberType Type::* Member){
    return reinterpret_cast<ptrdiff_t>(reinterpret_cast<const char*>(&(reinterpret_cast<Type*>(8)->*Member)) - 8);
};

template<class Type, typename MemberType>
inline const Type* container_of(MemberType* ptr, MemberType Type::* Member){
    auto offset = offset_of<Type, MemberType>(Member);
    return reinterpret_cast<const Type*>(reinterpret_cast<const char*>(ptr) - offset);
};

#endif /* defined(__uv__wrapper__) */
