//
//  base64.inl
//  uv
//
//  Created by Christopher Tarquini on 10/16/13.
//  Copyright (c) 2013 Christopher Tarquini. All rights reserved.
//

#ifndef __uv__base64__
#define __uv__base64__

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
}

template <typename TypeName>
size_t base64_decoded_size(const TypeName* src, size_t size) {
    if (size == 0)
        return 0;
    
    if (src[size - 1] == '=')
        size--;
    if (size > 0 && src[size - 1] == '=')
        size--;
    
    return base64_decoded_size_fast(size);
}


// supports regular and URL-safe base64
static const int unbase64_table[] =
{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -2, -1, -1, -2, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, 62, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, 63,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
#define unbase64(x) unbase64_table[(uint8_t)(x)]


template <typename TypeName>
size_t base64_decode(char* buf,
                     size_t len,
                     const TypeName* src,
                     const size_t srcLen) {
    char a, b, c, d;
    char* dst = buf;
    char* dstEnd = buf + len;
    const TypeName* srcEnd = src + srcLen;
    
    while (src < srcEnd && dst < dstEnd) {
        int remaining = srcEnd - src;
        
        while (unbase64(*src) < 0 && src < srcEnd) src++, remaining--;
        if (remaining == 0 || *src == '=') break;
        a = unbase64(*src++);
        
        while (unbase64(*src) < 0 && src < srcEnd) src++, remaining--;
        if (remaining <= 1 || *src == '=') break;
        b = unbase64(*src++);
        
        *dst++ = (a << 2) | ((b & 0x30) >> 4);
        if (dst == dstEnd) break;
        
        while (unbase64(*src) < 0 && src < srcEnd) src++, remaining--;
        if (remaining <= 2 || *src == '=') break;
        c = unbase64(*src++);
        
        *dst++ = ((b & 0x0F) << 4) | ((c & 0x3C) >> 2);
        if (dst == dstEnd) break;
        
        while (unbase64(*src) < 0 && src < srcEnd) src++, remaining--;
        if (remaining <= 3 || *src == '=') break;
        d = unbase64(*src++);
        
        *dst++ = ((c & 0x03) << 6) | (d & 0x3F);
    }
    
    return dst - buf;
}


#endif /* defined(__uv__base64__) */
