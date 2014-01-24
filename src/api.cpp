//
//  api.cpp
//  uv
//
//  Created by Christopher Tarquini on 1/23/14.
//  Copyright (c) 2014 Christopher Tarquini. All rights reserved.
//

#ifndef uv_api_cpp
#define uv_api_cpp

#include "v8.h"

v8::internal::Thread::LocalStorageKey v8::internal::PerThreadAssertScopeBase::thread_local_key;
void v8::internal::FatalProcessOutOfMemory(char const* location) {
    FATAL("Process out of memory");
    FATAL(location);
}

#endif
