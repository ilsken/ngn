//
//  buffer.cpp
//  uv
//
//  Created by Christopher Tarquini on 10/10/13.
//  Copyright (c) 2013 Christopher Tarquini. All rights reserved.
//

#include "buffer.h"
#include <iostream>
#include <algorithm>

namespace ngn {
    
    typedef Buffer::iterator buffer_iterator;
    typedef Buffer::const_iterator const_buffer_iterator;
    typedef Buffer::reference reference;
    typedef Buffer::const_reference const_reference;
    typedef Buffer::value_type value_type;
    typedef Buffer::difference_type difference_type;
    
    //
    // Constructors
    //
    // By default, Buffers constuctor will copy data passed to it
    // Memory will automatically be freed when all related buffers are freed
    //
    Buffer::Buffer()
    : buffer_(uv_buf_init(0, 0)), base_(nullptr) {};

    Buffer::Buffer(size_type size)
        : buffer_(uv_buf_init(new char[size], static_cast<unsigned int>(size)))
    , base_(std::shared_ptr<value_type>(reinterpret_cast<pointer>(buffer_.base), std::default_delete<value_type[]>()))
    {};
    
    // take ownership of unique_ptrs
    Buffer::Buffer(std::unique_ptr<value_type[]> data, size_type offset, size_type length): Buffer(std::shared_ptr<value_type>(data.release(), data.get_deleter()), offset, length) {};
    
    Buffer::Buffer(std::shared_ptr<value_type> data, size_type offset, size_type length)
    : buffer_(uv_buf_init(reinterpret_cast<char*>(&data.get()[offset]), static_cast<unsigned int>(length))), base_(data){
    }
    
    Buffer::Buffer(const char* data, size_type length) : Buffer(data, 0, length) {};
    Buffer::Buffer(const char* data, size_type offset, size_type length) : Buffer(length - offset) {
        std::copy_n(&data[offset], length, buffer_.base);
    };
    
    // copy constructor
    Buffer::Buffer(const Buffer& other) : Buffer(other.size()) {
        std::copy_n(other.buffer_.base, buffer_.len, buffer_.base);
    };
    
    // move constructor
    Buffer::Buffer(Buffer&& other) noexcept {
        using std::swap;
        swap(base_, other.base_);
        swap(buffer_, other.buffer_);
    }
    
    //
    // External Constructors
    //
    Buffer Buffer::External(uv_buf_t buf){
        Buffer ret;
        ret.buffer_ = buf;
        return ret;
    };
    
    // external from char*
    Buffer Buffer::External(char* data, size_type offset, size_type length){
        return Buffer::External(uv_buf_init(&data[offset], static_cast<unsigned int>(length)));
    };
    Buffer Buffer::External(char* data, size_type length){ return Buffer::External(data, 0, length); };
    Buffer Buffer::External(char* data){ return Buffer::External(data, 0, strlen(data));};
    
    //
    // Modifiers
    //
    void Buffer::fill(const_reference c){ fill(c, begin(), end()); };
    void Buffer::fill(const_reference c, buffer_iterator begin){ fill(c, begin, end()); };
    void Buffer::fill(const_reference c, buffer_iterator begin, buffer_iterator end){
        std::fill(begin, end, c);
    }
    void Buffer::fill_n(const_reference c, buffer_iterator begin, size_type size){
        std::fill_n(begin, size, c);
    }
    
    Buffer Buffer::slice(iterator begin, iterator end) const{
        Buffer buf;
        buf.base_ = base_;
        buf.buffer_ = uv_buf_init(reinterpret_cast<char*>(&(*begin)), static_cast<unsigned int>(std::distance(begin, end)));
        return buf;
    };
    
    //
    // Operators
    //
    Buffer& Buffer::operator=(const Buffer& rhs) {
        if (*const_cast<const Buffer*>(this) != rhs){
            base_ = rhs.base_;
            buffer_ = rhs.buffer_;
        }
        return *this;
    };
    
    Buffer& Buffer::operator=(Buffer&& rhs){
        using std::swap;
        if(*this != rhs){
            swap(base_, rhs.base_);
            swap(buffer_, rhs.buffer_);
        }
        return *this;
    }

    bool Buffer::operator==(const Buffer& rhs) const{
        return buffer_.base == rhs.buffer_.base && buffer_.len == rhs.buffer_.len && base_ == rhs.base_;
    };
    bool Buffer::operator!=(const Buffer& rhs) const{
        return !(*this == rhs);
    };
    
    const_reference Buffer::operator[](size_t index) const{
        return reinterpret_cast<const_reference>(buffer_.base[index]);
    };
    
    reference Buffer::operator[](size_t index){
        return reinterpret_cast<reference>(buffer_.base[index]);
    };
    
    //
    // Container implentation
    //
    
    // Element access
    reference Buffer::at(Buffer::size_type index) {
        if (index < buffer_.len) return (*this)[index];
        else throw std::out_of_range("index out of range");
    };
    
    const_reference Buffer::at(Buffer::size_type index) const {
        if (index < buffer_.len) return (*this)[index];
        else throw std::out_of_range("index out of range");
    };
    
    // Capacity
    Buffer::size_type Buffer::size() const noexcept{
        return buffer_.len;
    };
    Buffer::size_type Buffer::max_size() const noexcept{
        return size();
    };
    bool Buffer::empty() const noexcept{
        return size() == 0;
    } ;
    void Buffer::reserve(Buffer::size_type size) {
        size_t len(buffer_.len);
        if (size > len) {
            char* newbuf = new char[size];
            if (buffer_.base != nullptr && len) {
                std::memcpy(newbuf, buffer_.base, len);
            }
            base_.reset(newbuf);
            buffer_ = uv_buf_init(newbuf, static_cast<unsigned int>(size));
        }
        
    }
    void Buffer::resize(Buffer::size_type size) {
        size_t len(buffer_.len);
        
        if (size > len) {
            reserve(size);
        } else if (size < len) {
            buffer_.len = size;
        }
    }
    void Buffer::resize(Buffer::size_type size, const Buffer::value_type val) {
        size_t len(buffer_.len);
        
        if (size > len) {
            char* base = buffer_.base + len;
            reserve(size);
            std::memset(base, val, buffer_.base - base);
        } else if (size < len) {
            buffer_.len = size;
        }
    }
    
    // Iterators
    Buffer::const_iterator Buffer::cbegin() const {
        return Buffer::const_iterator(&(*this)[0]);
    };
    const_buffer_iterator Buffer::cend() const {
        return Buffer::const_iterator(&(*this)[buffer_.len]);
    };
    Buffer::const_reverse_iterator Buffer::crbegin() const {
        return Buffer::const_reverse_iterator(cend());
    };
    Buffer::const_reverse_iterator Buffer::crend() const {
        return Buffer::const_reverse_iterator(cbegin());
    };
    Buffer::iterator Buffer::begin() {
        return Buffer::iterator(&(*this)[0]);
    };
    Buffer::const_iterator Buffer::begin() const{
        return cbegin();
    };
    Buffer::iterator Buffer::end() {
        return Buffer::iterator(&(*this)[buffer_.len]);
    };
    Buffer::const_iterator Buffer::end() const{
        return cend();
    };
    Buffer::reverse_iterator Buffer::rbegin() {
        return Buffer::reverse_iterator(end());
    };
    Buffer::reverse_iterator Buffer::rend() {
        return Buffer::reverse_iterator(begin());
    };
    
};