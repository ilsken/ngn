// Modified from https://github.com/joyent/node
//
// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef SRC_STRING_BYTES_H_
#define SRC_STRING_BYTES_H_
#include <string>
#include <assert.h>
#include <iostream>
#include "ngn.h"
#include "buffer.h"
// Decodes a v8::Handle<v8::String> or Buffer to a raw char*
namespace std {
    template <>
    class codecvt<char, char, char> {
        
    };
}
template <typename TypeName>
unsigned hex2bin(TypeName c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return 10 + (c - 'A');
    if (c >= 'a' && c <= 'f')
        return 10 + (c - 'a');
    return static_cast<unsigned>(-1);
}
//// Base 64 ////

#define base64_encoded_size(size) ((size + 2 - ((size + 2) % 3)) / 3 * 4)


// Doesn't check for padding at the end.  Can be 1-2 bytes over.
static inline size_t base64_decoded_size_fast(size_t size) {
    size_t remainder = size % 4;
    
    size = (size / 4) * 3;
    if (remainder) {
        if (size == 0 && remainder == 1) {
            // special case: 1-byte input cannot be decoded
            size = 0;
        } else {
            // non-padded input, add 1 or 2 extra bytes
            size += 1 + (remainder == 3);
        }
    }
    
    return size;
};

template <typename TypeName>
size_t base64_decoded_size(const TypeName* src, size_t size) {
    if (size == 0)
        return 0;
    
    if (src[size - 1] == '=')
        size--;
    if (size > 0 && src[size - 1] == '=')
        size--;
    
    return base64_decoded_size_fast(size);
};

namespace ngn {
    class StringBytes {
    public:
        // Does the string match the encoding? Quick but non-exhaustive.
        // Example: a HEX string must have a length that's a multiple of two.
        // FIXME(bnoordhuis) IsMaybeValidString()? Naming things is hard...
        static bool IsValidString(std::string string, enum encoding enc);
        
        // Fast, but can be 2 bytes oversized for Base64, and
        // as much as triple UTF-8 strings <= 65536 chars in length
        static size_t StorageSize(const std::string& val, enum encoding enc);
        
        // Precise byte count, but slightly slower for Base64 and
        // very much slower for UTF-8
        static size_t Size(const std::string& val, enum encoding enc);
        
        
        // Write the bytes from the string or buffer into the char*
        // returns the number of bytes written, which will always be
        // <= buflen.  Use StorageSize/Size first to know how much
        // memory to allocate.
        static size_t Write(char* buf,
                            size_t buflen,
                            const Buffer& val,
                            enum encoding encoding,
                            size_t* chars_written = nullptr);
        
        // Take the bytes in the src, and turn it into a Buffer or String.
        static Buffer Encode(const char* buf,
                                           size_t buflen,
                                           enum encoding encoding);
    };
    class codecvt_base64
    : public std::codecvt<char, char, std::mbstate_t> {
        typedef std::codecvt<char, char, std::mbstate_t> Base;
    public:
        
        codecvt_base64(size_t refs = 0) {}
        int storage_size(state_type& state,
                         const intern_type* to,
                         const intern_type* to_end,
                         int max) const{
            return std::min<int>(max, base64_encoded_size(to_end - to));
        };
    protected:
        union state_impl_t {
            state_type state;
            struct p_impl{
                char last_chars[3];
                unsigned count = 0;
            } impl;
        };
       
        static_assert(sizeof(state_impl_t) == sizeof(state_type),
                      "state impl must be the same size as state_type");
        // bytes -> base64
        result do_out( state_type& state,
                      const intern_type* from,
                      const intern_type* from_end,
                      const intern_type*& from_next,
                      extern_type* to,
                      extern_type* to_end,
                      extern_type*& to_next ) const {
            from_next = from;
            to_next = to;
            static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";
            state_impl_t::p_impl* m_state = &(reinterpret_cast<state_impl_t&>(state)).impl;
            const bool has_state = m_state->count > 0;
            
            unsigned a;
            unsigned b;
            unsigned c;
     
            if (has_state) {
                while (from_next < from_end && m_state->count <= 3) {
                    m_state->last_chars[(m_state->count++)] = *(from_next++) & 0xff;
                }
                assert(m_state->count >= 0 && m_state->count <= 3);
                if (m_state->count <= 3) {
                    // didn't fill the buffer yet
                    return partial;
                } else {
                    if (to_next + 3 < to_end) {
                        a = m_state->last_chars[0];
                        b = m_state->last_chars[1];
                        c = m_state->last_chars[2];
                        
                        *(to_next++) = table[a >> 2];
                        *(to_next++) = table[((a & 3) << 4) | (b >> 4)];
                        *(to_next++) = table[((b & 0x0f) << 2) | (c >> 6)];
                        *(to_next++) = table[c & 0x3f];
                        
                        // reset state
                        m_state->count = 0;
                    } else {
                        // not enough space in the destination buffer
                        return partial;
                    }
                }
            }
            
            while (from_next + 2 < from_end && to_next + 3 < to_end) {
                a = *(from_next++) & 0xff;
                b = *(from_next++) & 0xff;
                c = *(from_next++) & 0xff;
                
                *(to_next++) = table[a >> 2];
                *(to_next++) = table[((a & 3) << 4) | (b >> 4)];
                *(to_next++) = table[((b & 0x0f) << 2) | (c >> 6)];
                *(to_next++) = table[c & 0x3f];
            }
            // get leftover chunks
            auto delta = from_end - from_next;
            if (delta <= 3 - m_state->count) {
                while (from_next < from_end && m_state) {
                    m_state->last_chars[(m_state->count++)] = *(from_next++);
                }
            }
            
            return from_next == from_end ? ok : partial;

        };
        // base64 -> bytes
        result do_in( state_type& state,
                     const extern_type* from,
                     const extern_type* from_end,
                     const extern_type*& from_next,
                     intern_type* to,
                     intern_type* to_end,
                     intern_type*& to_next ) const {
            from_next = from;
            to_next = to;
            
        };
        int do_length( state_type& state,
                      const extern_type* from,
                      const extern_type* from_end,
                      std::size_t max ) const {
            return std::min<int>(max, base64_decoded_size(from, from_end - from));
        };
        int do_max_length() const noexcept {
            return 3;
        };
        
        int do_encoding() const noexcept {
            return -1;
        };
        
        bool do_always_noconv() const noexcept {
            return false;
        };
        result do_unshift(state_type& state,
                          extern_type* to,
                          extern_type* to_end,
                          extern_type*& to_next) const {
            state_impl_t::p_impl* m_state = &(reinterpret_cast<state_impl_t&>(state)).impl;
            if (m_state->count == 0) return ok;

            unsigned a = m_state->last_chars[0];
            unsigned b = m_state->last_chars[1];
            unsigned c = m_state->last_chars[2];
            
            static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";
            to_next = to;
            if (to_next + 3 < to_end) {
                switch (m_state->count) {
                    case 1:
                        *(to_next++) = table[a >> 2];
                        *(to_next++) = table[(a & 3) << 4];
                        *(to_next++) = '=';
                        *(to_next++) = '=';
                        break;
                    case 2:
                        *(to_next++) = table[a >> 2];
                        *(to_next++) = table[((a & 3) << 4) | (b >> 4)];
                        *(to_next++) = table[(b & 0x0f) << 2];
                        *(to_next++) = '=';
                        break;
                    case 3:
                        *(to_next++) = table[a >> 2];
                        *(to_next++) = table[((a & 3) << 4) | (b >> 4)];
                        *(to_next++) = table[((b & 0x0f) << 2) | (c >> 6)];
                        *(to_next++) = table[c & 0x3f];
                        break;
                }
                m_state->count = 0;
            } else {
                return partial;
            }
            return ok;
        };
        
    };

    class codecvt_hex
    : public std::codecvt<char, char, std::mbstate_t> {
        typedef std::codecvt<char, char, std::mbstate_t> Base;
    public:
        
        codecvt_hex(size_t refs = 0) {
            
        }
    protected:
        union state_impl_t {
            state_type state;
            char last_char;
        };
        static_assert(sizeof(state_impl_t) == sizeof(state_type),
                      "state impl must be the same size as state_type");
        // bytes -> hex
        result do_out( state_type& state,
                      const intern_type* from,
                      const intern_type* from_end,
                      const intern_type*& from_next,
                      extern_type* to,
                      extern_type* to_end,
                      extern_type*& to_next ) const {
            from_next = from;
            to_next = to;
            while (from_next != from_end && to_next + 1 < to_end) {
                static const char hex[] = "0123456789abcdef";
                uint8_t val = static_cast<uint8_t>(*(from_next++));
                *(to_next++) = hex[val >> 4];
                *(to_next++) = hex[val & 15];
            }
            return from_next != from_end ? result:: partial : result::ok;
        };
        // hex -> bytes
        result do_in( state_type& state,
                     const extern_type* from,
                     const extern_type* from_end,
                     const extern_type*& from_next,
                     intern_type* to,
                     intern_type* to_end,
                     intern_type*& to_next ) const {
            from_next = from;
            to_next = to;
            if (!std::mbsinit(&state)) {
                if (from_next != from_end && to_next != to_end) {
                    unsigned a = hex2bin(reinterpret_cast<state_impl_t&>(state).last_char);
                    unsigned b = hex2bin(*(from_next++));
                    auto i = !~b;
                    if (!~a || i) {
                        from_next -= i;
                        return result::error;
                    }
                    *(to_next++) = a * 16 + b;
                    memset(&state, 0, sizeof(state_type));
                }
            }
            while (from_next + 1 < from_end && to_next != to_end) {
                unsigned a = hex2bin(*(from_next++));
                unsigned b = hex2bin(*(from_next++));
                auto i = (!~a) + (!~b);
                if (i > 0) {
                    from_next -= i;
                    // invalid hex
                    return error;
                }
                *(to_next++) = a * 16 + b;
            }
            if (std::distance(from_next, from_end) == 1) {
                reinterpret_cast<state_impl_t&>(state).last_char = *(from_next++);
                return partial;
            }
            return from_next == from_end ? ok : partial;
        };
        int do_length( state_type& state,
                      const extern_type* from,
                      const extern_type* from_end,
                      std::size_t max ) const {
            return std::min<int>(max, std::distance(from, from_end) / 2);
        }
        int do_max_length() const noexcept {
            return 1;
        };
        
        int do_encoding() const noexcept {
            return 2;
        };
        
        bool do_always_noconv() const noexcept {
            return false;
        };
        result do_unshift(state_type& state,
                          extern_type* to,
                          extern_type* to_end,
                          extern_type*& to_next) const {
            return !std::mbsinit(&state) ? ok : error;
        };
        
    };
    
}  // namespace node

#endif  // SRC_STRING_BYTES_H_