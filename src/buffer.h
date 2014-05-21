//
//  buffer.h
//  uv
//
//  Created by Christopher Tarquini on 10/10/13.
//  Copyright (c) 2013 Christopher Tarquini. All rights reserved.
//

#ifndef uv_buffer_h
#define uv_buffer_h

#include <vector>
#include <string>
#include <algorithm>
#include <type_traits>
#include <codecvt>
#include "exceptions.h"

#include <uv.h>
#include "pointer_iterator.h"
#include <iostream>
#include <array>
#include <numeric>

namespace ngn {
    using std::string;
    using std::vector;
    
    static constexpr bool IS_BIG_ENDIAN = htonl(47) == 47;
    enum ByteOrder {
        Default = BYTE_ORDER,
        LittleEndian = LITTLE_ENDIAN,
        BigEndian = BIG_ENDIAN
    };
    
    static inline void swizzle(char* start, size_t len) {
        char* end = start + len - 1;
        while (start < end) {
            char tmp = *start;
            *start++ = *end;
            *end-- = tmp;
        }
    }
    
    
    template <class T, class R>
    struct enable_if_trivially_copyable
    : std::enable_if<std::is_trivially_copyable<T>::value, R> {};
    
    template <class T, class T2,  class R>
    struct enable_if_same
    : std::enable_if<std::is_same<T, T2>::value, R> {};
    
    template <class T, class R, R (T::*member) () const>
    R get_member(const T& object) {
        return (object.*member)();
    };
    template <class T, class R, R T::*member>
    R get_member(const T& object) {
        return (object.*member);
    };
    
    template <class container, class iterator>
    container concat(iterator begin, iterator end, size_t size) {
        typedef typename container::size_type size_type;
        typedef typename iterator::value_type value_type;
        typedef typename container::iterator output_iterator;
        container ret;
        ret.resize(size);
        auto output = ret.begin();
        std::for_each(begin, end, [&ret, &output] (value_type& value) {
            output = std::copy(value.begin(), value.end(), output);
        });
        return ret;
    }
    template <class container, class iterator>
    container concat(iterator begin, iterator end) {
        typedef typename iterator::value_type value_type;
        size_t size = 0;
        std::for_each(begin, end, [&size] (value_type& value) {
            size += value.size();
        });
        return concat<container, iterator>(begin, end, size);
    }
    
    template <class container>
    container concat(container list, size_t size) {
        typedef typename container::iterator iterator;
        return concat<container, iterator>(list.begin(), list.end(), size);
    }
    
    template <class container, class list_t>
    container concat(list_t list) {
        typedef typename list_t::iterator iterator;
        return concat<container, iterator>(list.begin(), list.end());
    }
    /*
     template <class Type,
     class MemberType, MemberType member, class ReturnType = decltype(((Type*)(nullptr)->*member))>
     typename std::enable_if<std::is_member_pointer<MemberType>::value, ReturnType>::type
     get_member(Type object) {
     return (object.*member);
     };
     */
    
    
    
    class Buffer {
    public:
        //
        // Iterator Types
        //
        typedef PointerIterator<char> iterator;
        typedef PointerIterator<const char> const_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
        typedef std::reverse_iterator<iterator> reverse_iterator;
        
        //
        // Value Types
        //
        typedef iterator::value_type value_type;
        typedef iterator::reference reference;
        typedef const iterator::reference const_reference;
        typedef iterator::pointer pointer;
        typedef const pointer const_pointer;
        
        //
        // Size/Difference Types
        //
        typedef size_t size_type;
        typedef iterator::difference_type difference_type;
        
        //
        // Static methods
        //
        template<typename iterator_type>
        static enable_if_same<typename iterator_type::value_type, Buffer, Buffer>
        concat(iterator_type begin, iterator_type end){
            return ngn::concat<Buffer>(begin, end);
        };
        
        //
        // Constructors
        //
        explicit Buffer(size_type size);
        Buffer(std::unique_ptr<value_type[]> data, size_type offset, size_type length);
        Buffer(std::shared_ptr<value_type> data, size_type offset, size_type length);
        Buffer(const char* data, size_type offset, size_type length);
        Buffer(const char* data, size_type length);
        
        
        template <class container_t,
        // hacky way to enable containers and size types
        typename = typename std::enable_if<std::is_arithmetic<container_t>::value == false && std::is_trivial<container_t>::value == false, container_t>::type>
        explicit Buffer(container_t data) : Buffer(data.begin(), data.end()) {
            
        }
        
        explicit Buffer(std::string str) : Buffer(str.begin(), str.end()){
            
        }
        
        
        // iterator constructor
        template<class iterator_type>
        Buffer(iterator_type begin, iterator_type end)
        : Buffer(std::distance(begin, end)) {
            std::copy(begin, end, this->begin());
        };
        
        // copy/move constructor
        Buffer(const Buffer& other);
        Buffer(Buffer&& other) noexcept;
        
        //
        // External Constructors
        //
        static Buffer External(uv_buf_t buf);
        static Buffer External(char* data, size_type offset, size_type length);
        static Buffer External(char* data, size_type length);
        static Buffer External(char* data);
        
        
        template<class T,
        ByteOrder order=ByteOrder::Default>
        enable_if_trivially_copyable<T, T>
        read(const_iterator begin) const{
            if (begin + sizeof(T) >= end())
                throw std::out_of_range("offset + length must not exceed buffer size");
            T val;
            std::memcpy(&val, &(*begin), size());
            
            if (order != ByteOrder::Default && sizeof(T) > 1)
                swizzle(reinterpret_cast<char*>(&val), sizeof(T));
            return val;
        };
        
        
        template<class iterator_type>
        size_type write(iterator_type  begin, iterator_type end) {
            return write(begin, end, this->begin());
        };
        
        template<class iterator_type>
        size_type write(iterator_type  begin, iterator_type end, iterator offset) {
            size_type bytes_written = std::min(std::distance(begin, end), std::distance(offset, this->end()));
            std::copy_n(begin, bytes_written, offset);
            return bytes_written;
        };
        
        template<class Type, ByteOrder order = ByteOrder::Default>
        void write(const Type& data, iterator offset){
            static_assert(std::is_pointer<Type>::value,
                          "length is required for pointer values");
            static_assert(std::is_trivially_copyable<Type>::value,
                          "type of data must be trivially copyable (plain old data only!)" );
            std::memcpy(&(*offset), &data, sizeof(data));
            if (order != ByteOrder::Default && sizeof(data) > 1) {
                swizzle(reinterpret_cast<char*>(&(*offset)), sizeof(data));
            }
        };
        
        //
        // Operators
        //
        bool operator==(const Buffer& rhs) const;
        bool operator!=(const Buffer& rhs) const;
        Buffer& operator=(const Buffer& rhs);
        Buffer& operator=(Buffer&& rhs);
        const_reference operator[](size_t index) const;
        reference operator[](size_t index);
        
        //
        // Container Methods
        //
        
        // Capacity
        size_type size() const noexcept;
        size_type max_size() const noexcept;
        bool empty() const noexcept;
        
        
        // Element Access
        reference at(size_type size);
        const_reference at(size_type size) const;
        value_type& front(size_type size);
        const_reference front(size_type size) const;
        
        // Iterators
        iterator begin();
        const_iterator begin() const;
        iterator end();
        const_iterator end() const;
        const_iterator cbegin() const;
        const_iterator cend() const;
        const_reverse_iterator crbegin() const;
        const_reverse_iterator crend() const;
        reverse_iterator rbegin();
        reverse_iterator rend();
        
        // Modifiers
        Buffer copy(iterator target_begin, iterator target_end, iterator source_begin, iterator source_end);
        
        void fill(const_reference c);
        void fill(const_reference c, iterator begin);
        void fill(const_reference c, iterator begin, iterator end);
        void fill_n(const_reference c, iterator begin, size_type size);
        
        Buffer slice(iterator begin, iterator end) const;
        template <class container, class iterator>
        friend container concat(iterator, iterator, size_type);
        friend Buffer concat<Buffer, typename vector<Buffer>::iterator>(typename vector<Buffer>::iterator, typename vector<Buffer>::iterator, size_type);
        operator uv_buf_t() {
            return buffer_;
        }
        std::shared_ptr<value_type> handle() {
            return base_;
        }
    private:
        // for concat support
        Buffer();
        void reserve(size_type);
        void resize(size_type size);
        void resize(Buffer::size_type size, const value_type);
        uv_buf_t buffer_;
        //        Encoding::Encoding encoding_;
        // required to cleanly dispose of the buffer
        std::shared_ptr<value_type> base_;
        
    };
    
    
    
};
#endif
