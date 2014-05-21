//
//  traits.h
//  uv
//
//  Created by Christopher Tarquini on 10/15/13.
//  Copyright (c) 2013 Christopher Tarquini. All rights reserved.
//

#ifndef uv_traits_h
#define uv_traits_h

namespace ngn {
    template<class BufferType, class ObjectType, bool ObjectMode>
    struct data_event_traits {    };
    
    template<class BufferType, class ObjectType>
    struct data_event_traits<BufferType, ObjectType, true>{ typedef ObjectType event_type; };
    
    template<class BufferType, class ObjectType>
    struct data_event_traits<BufferType, ObjectType, false>{ typedef BufferType event_type; };
    
    template <class T>
    using value_type_t = typename T::value_type;
};

#endif
