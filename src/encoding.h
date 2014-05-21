//
//  encoding.h
//  uv
//
//  Created by Christopher Tarquini on 10/10/13.
//  Copyright (c) 2013 Christopher Tarquini. All rights reserved.
//

#ifndef uv_encoding_h
#define uv_encoding_h

#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include "buffer.h"

namespace ngn {
    namespace encoding {
        enum Encoding {
            Binary,
            ASCII,
            UTF8,
            USC2,
            Base64,
            Hex
        };
        static size_t hex_encode(const char* src, size_t slen, char* dst, size_t dlen)   {            // We know how much we'll write, just make sure that there's space.
            assert(dlen >= slen * 2 &&
                   "not enough space provided for hex encode");
            
            dlen = slen * 2;
            for (uint32_t i = 0, k = 0; k < dlen; i += 1, k += 2) {
                static const char hex[] = "0123456789abcdef";
                uint8_t val = static_cast<uint8_t>(src[i]);
                dst[k + 0] = hex[val >> 4];
                dst[k + 1] = hex[val & 15];
            }
            
            return dlen;
        }
        template <typename TypeName>
        size_t hex_decode(char* buf,
                          size_t len,
                          const TypeName* src,
                          const size_t srcLen) {
            size_t i;
            for (i = 0; i < len && i * 2 + 1 < srcLen; ++i) {
                unsigned a = hex2bin(src[i * 2 + 0]);
                unsigned b = hex2bin(src[i * 2 + 1]);
                if (!~a || !~b)
                    return i;
                buf[i] = a * 16 + b;
            }
            
            return i;
        }

        template <enum Encoding encoding>
        struct encoder {};
        
        template <>
        struct encoder<Encoding::Hex> {
            typedef encoder<Encoding::Hex> type;
            static const enum Encoding encoding = Hex;
            static size_t storageSize(const Buffer& buffer) {
                return buffer.size() * 2;
            }
            static const std::string encode(const Buffer& buffer) {
                std::string str;
                str.resize(type::storageSize(buffer));
                std::cout << "storage size: " << type::storageSize(buffer) << "\n";
                std::cout << "str size: " << str.size() << "\n";
                auto size = hex_encode(reinterpret_cast<const char *>(&(*buffer.cbegin())),
                                       buffer.size(), &str[0], str.size());
                str.resize(size);
                return str;
            };
            static const Buffer decode(const std::string& str) {
                // decode it
                return Buffer{str.begin(), str.end()};
            }
        };
        
        

    }
}
#endif
