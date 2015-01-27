//
//  buffer_decoder.h
//  ngn
//
//  Created by Christopher Tarquini on 1/28/14.
//
//

#ifndef __ngn__buffer_decoder__
#define __ngn__buffer_decoder__

#include <iostream>
#include <codecvt>
#include <vector>
#include <string.h>
#include "buffer.h"
#include <core/optional.hpp>

namespace ngn {
    enum Encoding {
        ASCII,
        UTF8,
        UTF16,
        UTF32,
        USC2,
        UTF16LE,
        BUFFER
    };
    
namespace internal {
    using ngn::Buffer;
    using std::string;
    using std::wstring;
    using std::u16string;
    using std::u32string;
    using core::optional;
    
    typedef std::string u8string;
    typedef std::string asciistring;
    template <
    class input_t,
    class output_t,
    class converter = std::codecvt<input_t, output_t, std::mbstate_t>>
    class StreamDecoder {
    public:
        StreamDecoder();
        StreamDecoder(const StreamDecoder<input_t, output_t, converter>&) = delete;
        
        ~StreamDecoder();
    private:
        uv_buf_t buffer;
    };
}
}
#endif /* defined(__ngn__buffer_decoder__) */
