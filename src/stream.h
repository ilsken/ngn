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
#include <list>
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
    
    template<>
    struct stream_traits<std::u16string>
    {
        typedef std::u16string chunk_type;
        typedef std::vector<Buffer> buffer_type;
        static const size_t high_water_mark = (16 * 1024) / 2;
        static const size_t buffer_reserve = 0;
        static const bool object_mode = false;
    };
    
    typedef stream_traits<std::string> ascii_stream_traits;
    typedef stream_traits<std::string> utf_stream_traits;
    typedef stream_traits<std::u16string> utf16_stream_traits;
    
    template <class ChunkType, class Traits, class Alloc> class WritableStreamBase;
    
    
    
    template <class T, class ListT>
    T splice(ListT& List, size_t size);
    
    template<>
    Buffer splice<Buffer, vector<Buffer>>(vector<Buffer>& list, size_t size) {
        Buffer buf(size);
        auto list_begin = list.begin();
        auto list_end = list.end();
        auto output_begin = buf.begin();
        auto output_end = buf.end();
        for(auto i = list_begin; i != list_end && output_begin != output_end; i++) {
            auto buf_size = i->size();
            auto chunk_size = std::min(buf_size, size);
            output_begin = std::copy_n(i->begin(), chunk_size, output_begin);
            // buffer is flushed
            if (chunk_size == buf_size) {
                list_begin++;
            } else {
                *i = i->slice(i->begin() + chunk_size, i->end());
            }
        }
        list.erase(list_begin, list_end);
        return buf;
    }
    
    
    template <class ChunkType = Buffer, class Traits = stream_traits<ChunkType>, class Alloc = std::allocator<ChunkType>>
    class ReadableStream {
        typedef ChunkType chunk_type;
        typedef Traits traits_type;
        typedef typename traits_type::buffer_type buffer_type;
        typedef size_t size_type;
        typedef Alloc allocator_type;
        
        ReadableStream();
        ReadableStream(allocator_type allocator);
        
        // no copy constructor
        ReadableStream(const ReadableStream&) = delete;
        
        // default move
        ReadableStream(ReadableStream&&) = default;
        
        // constants
        static const bool object_mode = traits_type::object_mode;
        
        // Events
        Event<> onReadable;
        Event<const chunk_type&> onData;
        Event<> onEnd;
        Event<> onClose;
        Event<std::exception> onError;
        
        optional<chunk_type> read(size_type n = 0) {
            auto orig = n;
            if (n > 0)
                is_readable_emitted = false;
            
            // if we're doing read(0) to trigger a readable event, but we
            // already have a bunch of data in the buffer, then just trigger
            // the 'readable' event and move on.
            if (n == 0 && needs_readable
                && (length_ > high_watermark || is_ended)) {
                if (length_ == 0 && is_ended) {
                    endReadable();
                } else {
                    emitReadable();
                }
                return nullopt;
            }
            
            n = howMuchToRead(n);
            
            if (n == 0 && is_ended) {
                if (length_ == 0) {
                    endReadable();
                }
            }
            // All the actual chunk generation logic needs to be
            // *below* the call to _read.  The reason is that in certain
            // synthetic stream cases, such as passthrough streams, _read
            // may be a completely synchronous operation which may change
            // the state of the read buffer, providing enough data when
            // before there was *not* enough.
            //
            // So, the steps are:
            // 1. Figure out what the state of things will be after we do
            // a read from the buffer.
            //
            // 2. If that resulting state will trigger a _read, then call _read.
            // Note that this may be asynchronous, or synchronous.  Yes, it is
            // deeply ugly to write APIs this way, but that still doesn't mean
            // that the Readable class should behave improperly, as streams are
            // designed to be sync/async agnostic.
            // Take note if the _read call is sync or async (ie, if the read call
            // has returned yet), so that we know whether or not it's safe to emit
            // 'readable' etc.
            //
            // 3. Actually pull the requested chunks out of the buffer and return.
            
            // if we currently have less than the highWaterMark, then also read some
            bool do_read = needs_readable;
            if (length_ == 0 || length_ - n < high_watermark) {
                do_read = true;
            }
            
            // however, if we've ended, then there's no point, and if we're already
            // reading, then it's unnecessary.
            if (is_ended || is_reading) {
                do_read = false;
            }
            if (do_read) {
                is_reading = true;
                is_sync = true;
                // if the length is currently zero, then we *need* a readable event.
                if (length_ == 0)
                    needs_readable = true;
                // call internal read method
                _read(high_watermark);
                is_sync = false;
            }
            
            // If _read pushed data synchronously, then `reading` will be false,
            // and we need to re-evaluate how much data we can return to the user.
            if (do_read && !is_reading)
                n = howMuchToRead(orig);
            
            optional<chunk_type> ret = n > 0 ? nullopt /* from list */ : nullopt;
            if (!ret) {
                needs_readable = true;
                n = 0;
            }
            length_ -= n;
            
            // If we have nothing in the buffer, then we want to know
            // as soon as we *do* get something into the buffer.
            if (length_ == 0 && !is_ended)
                needs_readable = true;
            
            // If we tried to read() past the EOF, then emit end on the next tick.
            if (orig != n && is_ended && length_ == 0) {
                endReadable();
            }
            
            if (orig)
                onData(*ret);
            
            return ret;
        }
        
        void resume();
        void pause();
        
        
    protected:
        void _read(size_t count);
    private:
        optional<chunk_type> fromList(size_type n) {
            typedef typename chunk_type::pointer chunk_pointer;
            if (buffer.empty())
                return nullopt;
            
            // assume buffer here
            n = std::min(n, length_);
            chunk_type ret(n);
            auto chunk_begin = ret.begin();
            auto chunk_end = ret.end();
            size_t flushed = 0;
            for (auto i : buffer) {
                size_t buf_size = i->size();
                size_t chunk_size = std::min(buf_size, n);
                std::copy_n(i->cbegin(), chunk_size, chunk_begin);
                chunk_begin += chunk_size;
                n -= chunk_size;
                if (buf_size == chunk_size) flushed += 1;
                else {
                    i = i->slice(i->cbegin() + chunk_size, i->cend());
                }
                if (chunk_begin >= chunk_end) break;
            }
            if (flushed)
                buffer.remove(buffer.begin(), buffer.begin()+flushed);
            return ret;
            
        }
        size_type howMuchToRead(size_type n) {
            if (length_ == 0 && is_ended)
                return 0;
            if (object_mode)
                return n == 0 ? 0 : 1;
            
            if (n > high_watermark)
                high_watermark = ngn::utils::next_power_of_2(high_watermark+1);
            
            if (n > length_) {
                if (!is_ended) {
                    needs_readable = true;
                    return 0;
                } else {
                    return length_;
                }
            }
            return n;
        }
        void endReadable() {
            assert(length_ > 0);
            if (!is_end_emitted) {
                is_ended = true;
                /* next tick */
                is_end_emitted = true;
                is_readable = false;
                onEnd();
            }
        }
        void emitReadable() {
            needs_readable = false;
            if (!is_readable_emitted) {
                is_readable_emitted = true;
                if (is_sync) {
                    /* nextTick */
                    emitReadableAndFlow();
                } else {
                    emitReadable();
                }
            }
        }
        
        void emitReadableAndFlow() {
            onReadable();
        }
        void flow() {
            if (is_flowing) {
                while (read());
            }
        }
        // allocator
        allocator_type allocator;
    
        bool is_flowing = false;
        bool is_end_emitted = false;
        bool is_ended = false;
        bool is_reading = false;
        size_t length_ = 0;
        size_t high_watermark = traits_type::high_watermark;
        
        // a flag to be able to tell if the onwrite cb is called immediately,
        // or on a later tick.  We set this to true at first, because any
        // actions that shouldn't happen until "later" should generally also
        // not happen before the first write call.
        bool is_sync = true;
        
        // whenever we return null, then we set a flag to say
        // that we're awaiting a 'readable' event emission.
        bool needs_readable = false;
        bool is_readable_emitted = false;
        bool is_readable_listening = false;
        bool is_readable = false;
        
        // when piping, we only care about 'readable' events that happen
        // after read()ing all the bytes and not getting any pushback.
        bool has_ran_out = false;
        
        // the number of writers that are awaiting a drain event in .pipe()s
        size_t awaiting_drain = 0;
        
        // if true, a maybeReadMore has been scheduled
        bool is_reading_more = false;
        
        buffer_type buffer;

        
    };
    template <class ChunkType = Buffer, class Traits = stream_traits<ChunkType>, class Alloc = std::allocator<ChunkType>>
    class WritableStreamBase {
        
    };
    /*
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
                return nullopt;
            }
            
            size_t available = get_available_size_(size);
            if (available == 0 && state == ReadableState::Ended) {
                if (buffer_size_ == 0) {
                    end_();
                }
                return nullopt;
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
            optional<chunk_type> ret;
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
            
            return nullopt;
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
                    event_loop_.nextTick([this] {
                        this->onReadable();
                        this->flow_();
                    });
                } else {
                    onReadable();
                    flow_();
                }
            }
        }
        
        void flow_() {
            if (state == ReadableState::Flowing) {
                optional<chunk_type> chunk;
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
        // state
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
    */
    
}
#endif /* defined(__uv__stream__) */
