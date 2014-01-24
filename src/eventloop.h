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



namespace ngn {
    typedef uv_run_mode RunMode;
    class EventLoop {
public:
        static const EventLoop& default_loop();
        
        EventLoop();
        EventLoop(uv_loop_t* handle);
        EventLoop(const EventLoop& that) = delete;
        ~EventLoop();
        
        typedef Event<> TickEvent;
        TickEvent onTick;
        
        template<class Type> Type* data() {
            return reinterpret_cast<Type*>(handle_->data);
        }
        template<class Type> void data(Type* data) {
            handle_->data = reinterpret_cast<void*>(data);
        }
        int run(RunMode mode);
        int run();
        void enableTick();
        template<class T>
        void addWork(std::function<T> fn);
        void stop();
        unsigned int active_handles();
        const uv_loop_t* const handle();

        
private:
        uv_loop_t* handle_;
        uv_idle_t* tick_handle_ = nullptr;
    
    };
   
}

#endif /* defined(__uv__eventloop__) */
