//
//  hex.inl
//  uv
//
//  Created by Christopher Tarquini on 10/16/13.
//  Copyright (c) 2013 Christopher Tarquini. All rights reserved.
//

#ifndef uv_hex_inl
#define uv_hex_inl
//// HEX ////

template <typename TypeName>
unsigned hex2bin(TypeName c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    return static_cast<unsigned>(-1);
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
        if (!~a || !~b) return i;
        buf[i] = a * 16 + b;
    }
    
    return i;
}


#endif
