//
//  exceptions.h
//  uv
//
//  Created by Christopher Tarquini on 10/10/13.
//  Copyright (c) 2013 Christopher Tarquini. All rights reserved.
//

#ifndef uv_exceptions_h
#define uv_exceptions_h

#include <string>
#include <sstream>

namespace ngn {
    using std::string;
    using std::exception;
    using std::stringstream;
    
    class BufferRangeException: public std::out_of_range
    {
    public:
        BufferRangeException(size_t buffer_size, size_t offset, size_t size) :std::out_of_range(""){
        buffer_size_ = buffer_size;
        offset_ = offset;
        size_ = size;
        stringstream message("Can not read ");
        message << size << (size == 1 ? "byte" : "bytes") << " at offset " << offset << " because it exceeds the buffer size (" << buffer_size << ")";
        message_ = message.str();
        
       };
       const size_t buffer_size(){
           return buffer_size_;
       };
       const size_t offset(){
           return offset_;
       };
       const size_t size(){
           return size_;
       }
       virtual const char* what() const throw()
       {
           return message_.c_str();
       }
        
    private:
       size_t buffer_size_;
       size_t offset_;
       size_t size_;
       string message_;
    };
}

#endif
