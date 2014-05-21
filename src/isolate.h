//
//  isolate.h
//  ngn
//
//  Created by Christopher Tarquini on 3/6/14.
//
//

#ifndef __ngn__isolate__
#define __ngn__isolate__

#include "eventloop.h"

#include <uv.h>
#include <thread>

namespace ngn {
    class isolate {
    public:
        class isolate_manager {
        public:
            isolate_manager() : tls_key(uv_key_t()) {
                uv_key_create(&tls_key);
            };
            inline void release(isolate* handle) {
                auto tls_isolate = get();
                assert(handle == tls_isolate);
                delete handle;
                uv_key_set(&tls_key, nullptr);
            }
            inline isolate* get(bool use_default_loop = false, bool create = false) {
                auto instance = reinterpret_cast<isolate*>(uv_key_get(&tls_key));
                if (instance == nullptr && create) {
                    instance = use_default_loop ? new isolate(uv_default_loop()) : new isolate();
                    uv_key_set(&tls_key, instance);
                }
                return instance;
            };
        private:
            uv_key_t tls_key;
        };

        
    public:
        static isolate& instance(bool use_default_loop = false) {
            static isolate_manager manager;
            return *manager.get(use_default_loop);
        };
        isolate(uv_loop_t* handle) : m_loop(handle), m_thread_id(std::this_thread::get_id()) {
            
        }
        isolate() :
            m_loop(EventLoop()),
            m_thread_id(std::this_thread::get_id()) {
        };
        isolate(const isolate&) = delete;
        isolate(isolate&&) = delete;
        
        std::thread::id thread_id() {
            return m_thread_id;
        }
        EventLoop& event_loop() {
            return m_loop;
        }
        ~isolate() {
            // allow event loop to cleanup
            m_loop.run();
            
        }
        
    private:
        EventLoop m_loop;
        const std::thread::id m_thread_id;
    };
}

#endif /* defined(__ngn__isolate__) */
