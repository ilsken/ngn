//
//  filesystem.h
//  uv
//
//  Created by Christopher Tarquini on 10/10/13.
//  Copyright (c) 2013 Christopher Tarquini. All rights reserved.
//

#ifndef __uv__filesystem__
#define __uv__filesystem__
#include <string>
#include <functional>

namespace ngn{
    using std::string;
    
    class FileSystem{
    public:
        enum Encoding{
            Buffer,
            UTF8,
            Hex
        };
        typedef int FileHandle;
        typedef std::function<void (std::exception, string)> readStringCallback;
        typedef std::function<void (std::exception, FileHandle)> openFileCallback;

        void openFile(string fileName, openFileCallback callback);
        void readFile(string fileName, Encoding encoding, readStringCallback callback);
        void writeFile(string fileName, Encoding encoding, const char* buffer, size_t length);
        void writeFile(string fileName, Encoding encoding, const char* buffer, size_t offset,  size_t length);
    };
}

#endif /* defined(__uv__filesystem__) */
