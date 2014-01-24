//
//  main.cpp
//  uv
//
//  Created by Christopher Tarquini on 10/10/13.
//  Copyright (c) 2013 Christopher Tarquini. All rights reserved.
//

#include <iostream>
#include "eventloop.h"
#include <uv.h>
#include "buffer.h"
#include <bitset>
#include <array>
using ngn::EventLoop;
using ngn::Buffer;
#include "wrapper.h"
#include "stream.h"
#include "optional.h"
using ngn::ReadableStream;
using ngn::WritableStream;
using ngn::encoding::encoder;
using ngn::encoding::Encoding;
#include <locale>

#include "scanner-character-streams.h"
namespace ngn {
    template<>
    template<>
    std::optional<std::string> ReadableStream<Buffer>::read(size_t size) {
        std::optional<std::string> ret;
        auto chunk = this->read<Buffer>(size);
        if (chunk)
            ret.emplace(chunk->begin(), chunk->end());
        return ret;
    }
}

int main(int argc, const char * argv[])
{
    EventLoop ev;
    char buf[] = "\xC3\xA6\xC3\xB0";
    size_t buflen = strlen(buf);
    unibrow::Utf8Decoder<128> decoder(buf, buflen);
    size_t outlen = decoder.Utf16Length();
    uint16_t* out = new uint16_t[outlen + 1];
    decoder.WriteUtf16(out, outlen);
    for(int i = 0; i < outlen; i++) {
        std::cout << std::hex << out[i];
    }

}


