//
//  stream.h
//  uv
//
//  Created by Christopher Tarquini on 10/10/13.
//  Copyright (c) 2013 Christopher Tarquini. All rights reserved.
//

#ifndef __uv__stream__
#define __uv__stream__

#include <functional>
#include <cmath>
#include "eventloop.h"
#include "event.h"
#include "encoding.h"
#include "buffer.h"
#include "traits.h"
#include "optional.h"
#include "utils.h"
#include "encoding.h"


// don't raise the hwm above 128MB
// see: https://github.com/joyent/node/blob/master/lib/_stream_readable.js#L205
#define UV_MAX_HWM 0x800000

namespace ngn{
    using std::string;
    using std::optional;
    
    enum ReadableState {
        Ended,
        Initial,
        Readable,
        Flowing,
        
    };
    
    
    template<class ChunkType>
    struct stream_traits {
        typedef std::vector<ChunkType> chunk_type;
        typedef chunk_type buffer_type;
        static const size_t high_water_mark = 16;
        static const size_t buffer_reserve = 0;
        static const bool object_mode = true;
    };
    
    template<>
    struct stream_traits<Buffer>{
        typedef Buffer chunk_type;
        typedef std::vector<Buffer> buffer_type;
        static const size_t high_water_mark = 16 * 1024;
        static const size_t buffer_reserve = 0;
        static const bool object_mode = false;
    };
    
    template<>
    struct stream_traits<std::string>
    {
        typedef std::string chunk_type;
        typedef std::vector<Buffer> buffer_type;
        static const size_t high_water_mark = 16 * 1024;
        static const size_t buffer_reserve = 0;
        static const bool object_mode = false;
    };
  

    
    template<class ChunkType, class Traits = stream_traits<ChunkType>>
    class ReadableStream {
    public:
        typedef typename Traits::buffer_type buffer_type;
        typedef ChunkType chunk_type;
        static constexpr bool object_mode = Traits::object_mode;

        Event<> onReadable;
        Event<chunk_type&> onData;
        Event<> onEnd;
        Event<> onClose;
        Event<std::exception> onError;
        
        
        ReadableStream(const EventLoop& loop = EventLoop::default_loop()) : event_loop_(loop) {
        }
        
        template <class ReturnType>
        optional<ReturnType> read(size_t size = 0) {
            if (size > 0) {
                emitted_readable_ = false;
            }
            if (size == 0 && need_readable_
                && (buffer_size_ >= high_water_mark_ || state == ReadableState::Ended)) {
                // if we're done emit the end event
                if (buffer_size_ == 0 && state == ReadableState::Ended) {
                    end_();
                } else {
                    readable_();
                }
                return std::nullopt;
            }
            
            size_t available = get_available_size_(size);
            if (available == 0 && state == ReadableState::Ended) {
                if (buffer_size_ == 0) {
                    end_();
                }
                return std::nullopt;
            }
            bool do_read = need_readable_;
            
            // if we currently have less than the highWaterMark, then also read some
            if (buffer_size_ == 0 || buffer_size_ - size < high_water_mark_) {
                do_read = true;
            }
            
            if (state == ReadableState::Ended || is_reading_) {
                do_read = false;
            }
            if (do_read) {
                is_reading_ = true;
                sync_ = true;
                if (buffer_size_ == 0)
                    need_readable_ = true;
                read_(high_water_mark_);
                sync_ = false;
                
            }
            std::optional<chunk_type> ret;
            if (do_read && !is_reading_) {
                available = get_available_size_(size);
            }
            if (available > 0) {
                // ret = fromList(number)
            }
            if (!ret) {
                need_readable_ = true;
                available = 0;
            }
            buffer_size_ -= available;
            if (size != available && state == Ended) {
                end_();
            }
            if (ret) {
                auto chunk = *ret;
                onData(chunk);
                return chunk;
            }
            
            return std::nullopt;
        };

        template<typename StreamType>
        StreamType pipe(StreamType destination);
        template<typename StreamType>
        StreamType unpipe(StreamType destination);
        
        
        virtual ~ReadableStream() = default;
        bool push_back(ChunkType chunk) {
            buffer_.push_back(chunk);
            onReadable();
            return true;
        };
        
        bool push_front(ChunkType chunk) {
            buffer_.push_front(chunk);
            return true;
        };
    protected:
        virtual void read_(size_t size = 0);
    private:
        
        void end_() {
            if (buffer_size_ > 0) {
                throw;
            }
            if (!end_emitted_) {
                state = ReadableState::Ended;
                
            }
        }
        void readable_() {
            need_readable_ = false;
            if (!emitted_readable_) {
                emitted_readable_ = true;
                if (sync_) {
                    // TODO: next tick
                    onReadable();
                    flow_();
                } else {
                    onReadable();
                    flow_();
                }
            }
        }
        
        void flow_() {
            if (state == ReadableState::Flowing) {
                std::optional<chunk_type> chunk;
                do {
                    chunk = this->read<chunk_type>(0);
                } while(chunk && state == ReadableState::Flowing);
            }
        }
        size_t get_available_size_(size_t size) {
            if (buffer_size_ == 0 && state == ReadableState::Ended) {
                return 0;
            }
            if (object_mode) {
                return size == 0 ? 0 : 1;
            }
            if (size <= 0) {
                return 0;
            }
            
            // scale up HWM
            if (size > high_water_mark_ && high_water_mark_ != UV_MAX_HWM) {
                high_water_mark_ = ngn::utils::next_power_of_2(high_water_mark_+1);
                if (high_water_mark_ > UV_MAX_HWM)
                    high_water_mark_ = UV_MAX_HWM;
            }
            if (size > buffer_size_) {
                if (state != ReadableState::Ended) {
                    need_readable_ = true;
                    return 0;
                } else {
                    return buffer_size_;
                }
            }
            return size;
        }
        /* state */
        size_t high_water_mark_ = Traits::high_water_mark;
        const EventLoop& event_loop_;
        bool emitted_readable_ = false;
        bool end_emitted_ = false;
        bool need_readable_ = true;
        bool is_reading_ = false;
        size_t buffer_size_ = 0;
        bool sync_ = false;
        ReadableState state = ReadableState::Initial;
        buffer_type buffer_;
    };
    
    template<class WriteType, class Traits = stream_traits<WriteType>>
    class WritableStream{
    public:
        typedef typename Traits::buffer_type buffer_type;
        typedef WriteType write_type;
        //typedef typename data_event_traits<buffer_type, write_type, Traits::object_mode>::event_type data_pipe_type;
        static constexpr bool object_mode = Traits::object_mode;
        
        typedef std::function<void()> writeCallback;
        Event<> onDrain;
        Event<> onFinish;
        //Event<ReadableStream<data_pipe_type>> onPipe;
       // Event<ReadableStream<data_pipe_type>> onUnpipe;
        Event<std::exception> onError;
        
        WritableStream(const EventLoop& loop = EventLoop::default_loop()) : event_loop_(loop){
            
        }
        
        template <class ChunkType>
        bool write(ChunkType chunk, writeCallback cb);
        template <class ChunkType>
        bool end(ChunkType chunk, writeCallback cb);
        
        void cork();
        void uncork();
        void reserve(size_t size);
        
        virtual ~WritableStream() = default;
        
        
    private:
        struct WriteRequest_{
            WriteType chunk;
            writeCallback callback;
        };
        bool sync_;
        bool is_writing_;
        bool is_finished_;
        bool is_buffer_processing_;
        bool need_drain_;
        int pending_callbacks_;
        const EventLoop& event_loop_;
        size_t length;
        std::vector<WriteRequest_> buffer_;
    };
    
    
}
#endif /* defined(__uv__stream__) */
