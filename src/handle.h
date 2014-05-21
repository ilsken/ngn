//
//  handle.h
//  ngn
//
//  Created by Christopher Tarquini on 2/7/14.
//
//

#ifndef __ngn__handle__
#define __ngn__handle__

#include <uv.h>
#include <memory>
#include <type_traits>
#include <cstdint>
#include <exception>
#include <chrono>
#include <stdio.h>
#include <cxxabi.h>
#include "utils.h"
#include "eventloop.h"
#include "io_buffer.h"
#include "isolate.h"



#define NGN_UV_CHECK(expr) { int result = expr; if (result < 0) throw UVException(result); }
#define NGN_MAKE_SHARED(T)
#define NGN_HANDLE_CONSTRUCTOR(T, H) \
    typedef HandleWrap<H> Wrapper; \
    T(isolate& isolate = isolate::instance()) : Wrapper(isolate)

namespace ngn{ namespace detail {
    void force_close(uv_handle_t*);
    bool is_closed(uv_handle_t*);
}}

namespace ngn {
    class UVException : std::exception {
    public:
        UVException(int errorCode)
        : errorCode_(errorCode) {
        };
        const char* what() const noexcept{
            return uv_err_name(errorCode_);
        }
        int errorCode() const noexcept {
            return errorCode_;
        }
        ~UVException() noexcept {};
    private:
        int errorCode_;
    };
#define NGN_GET_HANDLE(ptr) reinterpret_cast<uv_handle_t*>(static_cast<T*>(ptr))
    template <typename T>
    class HandleWrap : protected T {
        typedef std::function<void(HandleWrap<T>*)> close_callback;
        static void on_close(uv_handle_t* handle) {
            HandleWrap<T>* ptr = static_cast<HandleWrap<T>*>(reinterpret_cast<T*>(handle));
            //HandleWrap<T>* ptr = utils::container_of<HandleWrap<T>, T>(reinterpret_cast<T*>(handle), &HandleWrap::handle_);
            ptr->close_cb(ptr);
        }
    public:
        explicit HandleWrap(isolate& isolate = isolate::instance()) noexcept : m_isolate(isolate) {};
        HandleWrap(const HandleWrap&) = delete;
        HandleWrap(const HandleWrap&&) = delete;
        typedef T value_type;
        typedef value_type* pointer;
        typedef const pointer const_pointer;
        typedef value_type& reference;
        void ref() {
            uv_ref(&static_cast<uv_handle_t&>(*this));
        }
        void unref() {
            uv_unref(&static_cast<uv_handle_t&>(*this));
        }
        bool is_active() const {
            return uv_is_active(&static_cast<uv_handle_t&>(*this));
        }
        bool is_closing() const {
            return uv_is_closing(&static_cast<const uv_handle_t&>(*this));
        }
        bool has_reference() const {
            return uv_has_ref(&static_cast<uv_handle_t&>(*this));
        };
        void close() {
            uv_close(&static_cast<uv_handle_t&>(*this), nullptr);
        }
        void close(close_callback callback) {
            close_cb = callback;
            uv_close(&static_cast<uv_handle_t&>(*this), on_close);
        }
        inline EventLoop& event_loop() {
            return m_isolate.event_loop();
        };

        operator const uv_handle_t&() const {
            return *reinterpret_cast<const uv_handle_t*>(static_cast<const T*>(const_cast<const HandleWrap*>(this)));
        }
        operator uv_handle_t&() {
            return *reinterpret_cast<uv_handle_t*>(static_cast<T*>(const_cast<HandleWrap*>(this)));
        }
        ~HandleWrap() {
            // DANGER WILL ROBINSON, DANGER! DANGER!
            // If this handle is disposed before the event loop gets a chance
            // to run it's __cleanup_handles loop bad things will happen
            // If you're lucky it'll just segfault, if you're not so lucky
            // The titans will be released from 
            
            if (!is_closing()) {
                close([] (HandleWrap* ptr){
            
                });
            }
        }
        
    protected:
        inline pointer handle(){
            return static_cast<T*>(this);
        };
  


    private:
        isolate& m_isolate;
        close_callback close_cb;
    };

    template <class T, class Alloc = std::allocator<char>>
    class StreamWrap : public HandleWrap<T> {
        typedef Alloc allocator_type;
        typedef std::function<void(const experimental::Buffer&, size_t)> read_callback;
        typedef std::function<void(int status)> write_callback;

        static void on_alloc(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
            auto stream = static_cast<StreamWrap*>(handle);
            stream->m_read_buffer = experimental::Buffer(suggested_size);
            buf->base = stream->m_read_buffer.data();
            buf->len = suggested_size;
            
        }
        static void on_read(uv_stream_t* handle, size_t nread, const uv_buf_t* buf) {
            auto stream = static_cast<StreamWrap*>(reinterpret_cast<uv_handle_t*>(handle));
            stream->readfn_(stream->m_read_buffer);
        }
        static void on_write(uv_write_t* handle, int status) {
            auto req = static_cast<WriteRequest*>(handle);
            NGN_UV_CHECK(status);
            req->fn(status);
            delete req;
        }
    public:
        StreamWrap(EventLoop& loop = EventLoop::default_loop())
        : HandleWrap<T>(loop) {
            
        }
        void read_start(read_callback fn) {
            NGN_UV_CHECK(uv_read_start(this, on_alloc, on_read));
        }
        void read_stop() {
            NGN_UV_CHECK(uv_read_stop(this));
        }
        
        void write(const experimental::Buffer& buffer, write_callback callback) {
            std::allocator<char> p;
            auto req = new WriteRequest(buffer, callback);
            uv_write(req, this, &req->buffer, 1, on_write);
        }
    protected:
        operator uv_stream_t&() const {
            return *reinterpret_cast<uv_stream_t*>(static_cast<T*>(const_cast<StreamWrap*>(this)));
        }
    private:
        class WriteRequest : public uv_write_t {
        public:
            WriteRequest(const experimental::Buffer& buf, write_callback cb) : buffer(buf), fn(cb) {}
            const experimental::Buffer buffer;
            const write_callback fn;
        };
        allocator_type allocator;

        experimental::Buffer m_read_buffer;
        read_callback readfn_;
    };
    
#undef NGN_GET_HANDLE
   
    class Timer :
    public HandleWrap<uv_timer_t> ,
    public std::enable_shared_from_this<Timer> {
        static void timer_callback(uv_timer_t* handle, int status) {
            static_cast<Timer*>(handle)->fn_();
        };
    public:
        typedef std::chrono::milliseconds milliseconds;
        typedef std::function<void()> callback;
        
        static Timer& setTimeout(callback fn, milliseconds timeout, isolate& isolate = isolate::instance()) {
            Timer* timer = new Timer(isolate);
            timer->start(fn, timeout);
            return *timer;
        }
        static Timer& setInterval(callback fn, milliseconds interval, isolate& loop = isolate::instance()) {
            Timer* timer = new Timer(loop);
            timer->start(fn, interval, interval);
            return *timer;
        }
        void start(callback fn, milliseconds timeout = milliseconds(0),
                   milliseconds repeat = milliseconds(0)) {
            fn_ = fn;
            handle()->data = &fn_;
            NGN_UV_CHECK(uv_timer_start(this,
                                        Timer::timer_callback, timeout.count(),
                                        repeat.count()));
        }
        void stop() {
            NGN_UV_CHECK(uv_timer_stop(this));
        }
        void again() {
            int result = uv_timer_again(this);
        }
        milliseconds repeat() {
            return milliseconds(uv_timer_get_repeat(this));
        }
        void repeat(milliseconds repeat) {
            return uv_timer_set_repeat(this, repeat.count());
        }
    //private:
       NGN_HANDLE_CONSTRUCTOR(Timer, uv_timer_t) {
            uv_timer_init(event_loop(), this);
            this->data = this;
       };
        callback fn_;
    };
    
    class Idler :
    public HandleWrap<uv_idle_t>,
    public std::enable_shared_from_this<Idler> {
        // trigger user callbacks
        static void idle_callback(uv_idle_t* handle, int status) {
            static_cast<Idler*>(handle)->fn_();
        }
    public:
        typedef std::function<void()> callback;
        void start(callback fn = nullptr) {
            fn_ = fn;
            NGN_UV_CHECK(uv_idle_start(this, idle_callback));
        }
        void stop() {
            NGN_UV_CHECK(uv_idle_stop(this));
        }

    //private:
        NGN_HANDLE_CONSTRUCTOR(Idler, uv_idle_t) {
            NGN_UV_CHECK(uv_idle_init(event_loop(), this));
        };
        callback fn_;

    };
    
    class Signal :
    public HandleWrap<uv_signal_t>,
    public std::enable_shared_from_this<Signal> {
        static void on_signal(uv_signal_t* handle, int status) {
            static_cast<Signal*>(handle)->fn_(status);
        }
    public:
        typedef std::function<void(int)> callback;
        NGN_HANDLE_CONSTRUCTOR(Signal, uv_signal_t) {
            NGN_UV_CHECK(uv_signal_init(event_loop(), this));
        }
        
        void start(callback fn, int signal) {
            fn_ = fn;
            NGN_UV_CHECK(uv_signal_start(this, on_signal, signal));
        }
        
        void stop() {
            NGN_UV_CHECK(uv_signal_stop(this));
        }
    private:
        callback fn_;
    };

    class Async :
    public HandleWrap<uv_async_t>,
    public std::enable_shared_from_this<Async> {
        static void on_async(uv_async_t* handle, int status) {
            auto async = static_cast<Async*>(handle);
            async->m_fn();
        }
    public:
        using Wrapper = HandleWrap<uv_async_t>;
        using callback = std::function<void()>;
        
        Async(callback fn, isolate& isolate = isolate::instance()) :  Wrapper(isolate), m_fn(fn) {
            uv_async_init(isolate.event_loop().handle(), this, on_async);
        }
        void send() {
            uv_async_send(this);
        }
    private:
        callback m_fn;
    };
    class message_source;
    class message_sink : private HandleWrap<uv_async_t> , public std::enable_shared_from_this<message_sink> {
        static void on_sink(uv_async_t* handle, int status) {
            auto sink = static_cast<message_sink*>(handle);
            if (!sink->is_empty) {
                assert(!sink->is_flushed);
                sink->is_flushed = false;
                for (auto fn : sink->m_pending)
                    fn();
                sink->m_pending.clear();
                sink->is_empty = true;
                sink->is_flushed = true;
            }
        }
    public:
        using callback = std::function<void()>;
        using queue = std::vector<callback>;
        message_sink(isolate& isolate = isolate::instance()) : HandleWrap<uv_async_t>(isolate), is_empty(true), is_flushed(true) {
            uv_async_init(isolate.event_loop().handle(), this, on_sink);
            unref();
        }
        friend class message_source;
        ~message_sink() {
            on_sink(this, 0);
        }
    private:
        void notify() {
            uv_async_send(this);
        }
        std::atomic_bool is_empty;
        std::atomic_bool is_flushed;
        queue m_pending;
        
    };
    
    class message_source : public std::enable_shared_from_this<message_source> {
        
    public:
        using callback = message_sink::callback;
        using queue = message_sink::queue;
        message_source(message_sink& sink, isolate& isolate = isolate::instance()) : m_sink(sink), m_send_idle(isolate){
            m_send_idle.start([=] { flush(); });
            m_send_idle.unref();
        }
        bool flush () {
            if (m_pending.size() > 0) {
                if (m_sink.is_flushed) {
                    assert(m_sink.m_pending.size() == 0);
                    m_sink.m_pending.swap(m_pending);
                    m_sink.is_flushed = false;
                    m_sink.notify();
                    return true;
                } else {
                    m_sink.notify();
                    return false;
                }
            } else {
                return true;
            }
        }
        void operator() (callback fn) {
            m_pending.push_back(fn);
            m_sink.is_empty = false;
            
        }
        ~message_source() {
            // drain messages
            while(!flush());
        }
    private:
        message_sink& m_sink;
        Idler m_send_idle;
        queue m_pending;
        
    };
}

#endif /* defined(__ngn__handle__) */
