//
//  eventloop.cpp
//  uv
//
//  Created by Christopher Tarquini on 10/10/13.
//  Copyright (c) 2013 Christopher Tarquini. All rights reserved.
//

#include "eventloop.h"
#include <memory>

namespace ngn{
    
    //static EventLoop* default_loop_ptr = nullptr;
    void tick(uv_idle_t* handle, int status) {
        EventLoop* loop = reinterpret_cast<EventLoop*>(handle->data);
        loop->nextTick();
        loop->nextTick.removeAllListeners();
    }
    EventLoop::EventLoop(uv_loop_t* handle) : handle_(handle) {
    };
    
    EventLoop::EventLoop() : EventLoop(uv_loop_new()) {
    };
    
    void EventLoop::stop() {
        uv_stop(handle_);
    }

    EventLoop& EventLoop::default_loop() {
        static EventLoop ev(uv_default_loop());
        return ev;
        //return *default_loop_ptr;
    }
    
    
    int EventLoop::run(){
        return run(RunMode::UV_RUN_DEFAULT);
    }
    
    int EventLoop::run(RunMode mode) {
        return uv_run(handle_, mode);
    }
    
    uv_loop_t* EventLoop::handle() const {
        return handle_;
    };
    
    EventLoop::~EventLoop() {
        if (handle_ != nullptr) {
            uv_run(handle_, UV_RUN_NOWAIT);
            uv_loop_delete(handle_);
            handle_ = nullptr;
        }
    }
    
   
}