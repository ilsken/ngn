//
//  eventloop.cpp
//  uv
//
//  Created by Christopher Tarquini on 10/10/13.
//  Copyright (c) 2013 Christopher Tarquini. All rights reserved.
//

#include "eventloop.h"

namespace ngn{
    static const EventLoop default_loop_(uv_default_loop());
    
    void tick(uv_idle_t* handle, int status) {
        EventLoop* loop = reinterpret_cast<EventLoop*>(handle->data);
        loop->onTick();
        loop->onTick.removeAllListeners();
    }
    
    EventLoop::EventLoop(uv_loop_t* handle) : handle_(handle) {
    };
    
    EventLoop::EventLoop() : handle_(uv_loop_new()) {

    };
    
    const EventLoop& EventLoop::default_loop() {
        return default_loop_;
    }
    
    int EventLoop::run(){
        return run(RunMode::UV_RUN_DEFAULT);
    }
    
    int EventLoop::run(RunMode mode) {
        return uv_run(handle_, mode);
    }
    
    void EventLoop::enableTick() {
        if (tick_handle_ != nullptr) return;
        uv_idle_init(handle_, tick_handle_);
        EventLoop* self = &*this;
        tick_handle_->data = self;
        uv_idle_start(tick_handle_, tick);
        uv_unref(reinterpret_cast<uv_handle_t*>(tick_handle_));
    }
    
    
    const uv_loop_t* const EventLoop::handle(){
        return handle_;
    };
    
    EventLoop::~EventLoop() {
        if (handle_ != nullptr) {
            if (tick_handle_ != nullptr) {
                uv_idle_stop(tick_handle_);
            }
            uv_stop(handle_);
            uv_loop_delete(handle_);
            delete tick_handle_;
            tick_handle_ = nullptr;
            handle_ = nullptr;
        }
    }
    
   
}