//
//  eventloop.h
//  uv
//
//  Created by Christopher Tarquini on 10/10/13.
//  Copyright (c) 2013 Christopher Tarquini. All rights reserved.
//

#ifndef __uv__eventloop__
#define __uv__eventloop__
#include <functional>
#include <uv.h>
#include <vector>
#include "event.h"
#include "optional.h"
#include <unordered_map>

namespace ngn {
    
    class EventLoop;
    class Timer;
    class Idler;
    class Signal;
    

    typedef uv_run_mode RunMode;
    class EventLoop {
public:
        static EventLoop& default_loop();
        static std::unique_ptr<EventLoop> Default();
        EventLoop();
        EventLoop(uv_loop_t* handle);
        EventLoop(const EventLoop& that) {
            handle_ = that.handle_;
        }
        ~EventLoop();
        
        typedef Event<> TickEvent;
        TickEvent nextTick;
        
        template<class Type> Type* data() {
            return reinterpret_cast<Type*>(handle_->data);
        }
        template<class Type> void data(Type* data) {
            handle_->data = reinterpret_cast<void*>(data);
        }

        int run(RunMode mode);
        int run();
        int runWithTick();
        void stop();
        unsigned int active_handles();
        uv_loop_t* handle() const;
        uv_idle_t tick_handle_;
        bool tick_enabled_ = false;
        int fd();
        friend class Timer;
        friend class Idler;
        friend class Signal;
        
     
private:
        operator uv_loop_t*() {
            return handle_;
        }
        uv_loop_t* handle_;

    
    };
    
    template <class T>
    void ref(const T& wrapper) {
        auto ptr = wrapper.handle();
        if (ptr != nullptr && ptr->loop != nullptr) uv_ref(ptr);
    }
    template <class T>
    void unref(const T& wrapper) {
        auto ptr = wrapper.handle();
        if (ptr != nullptr && ptr->loop != nullptr) uv_unref(ptr);
    }
   
}

#endif /* defined(__uv__eventloop__) */
