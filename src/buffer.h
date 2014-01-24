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
#include "exceptions.h"

#include <uv.h>
#include "pointer_iterator.h"
#include <iostream>
#include <array>

namespace ngn {
    using std::string;
    using std::vector;
    
    
    static const bool IS_BIG_ENDIAN = htonl(47) == 47;
    enum ByteOrder {
        Default = BYTE_ORDER,
        LittleEndian = LITTLE_ENDIAN,
        BigEndian = BIG_ENDIAN
    };
    
    static inline void Swizzle(char* start, size_t len) {
        char* end = start + len - 1;
        while (start < end) {
            char tmp = *start;
            *start++ = *end;
            *end-- = tmp;
        }
    }


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
        static Buffer concat(iterator_type begin, iterator_type end){
            size_type size = 0;
            std::for_each(begin, end, [&size] (Buffer buffer){
                size += buffer.size();
            });
            auto data = new value_type[size];
            difference_type i = 0;
            
            std::for_each(begin, end, [&data, &i] (Buffer buffer){
                std::copy(buffer.begin(), buffer.end(), &data[i]);
                i += buffer.size();
            });
            return Buffer(std::unique_ptr<value_type[]>(data), 0l, size);
        };
        
        //
        // Constructors
        //
        explicit Buffer(size_type size);
        Buffer(std::unique_ptr<value_type[]> data, size_type offset, size_type length);
        Buffer(std::shared_ptr<value_type> data, size_type offset, size_type length);
        Buffer(const char* data, size_type offset, size_type length);
        Buffer(const char* data, size_type length);
        Buffer(const char* data);
        
        
        // iterator constructor
        template<class iterator_type>
        Buffer(iterator_type begin, iterator_type end)
        : Buffer(std::distance(begin, end)) {
            std::copy(begin, end, this->begin());
        };
        
        // copy/move constructor
        Buffer(const Buffer& other);
        Buffer(Buffer&& other);
        
        //
        // External Constructors
        //
        static Buffer External(uv_buf_t buf);
        static Buffer External(char* data, size_type offset, size_type length);
        static Buffer External(char* data, size_type length);
        static Buffer External(char* data);
        
        // external stl container
        template<class container>
        static Buffer External(container data) {
            return Buffer::External(data.begin(), data.end());
        };
        
        // external iterator
        template<class iterator_type>
        static Buffer External(iterator_type begin, iterator_type end) {
            return Buffer::External(uv_buf_init(&(*begin), std::distance(begin, end)));
        };
        

        template<class value_type,
                ByteOrder order=ByteOrder::Default,
                size_t length = sizeof(value_type),
                bool swap_bytes_ = order != ByteOrder::Default>
        typename std::enable_if<std::is_trivially_copyable<value_type>::value, value_type>::type
        read(const_iterator begin) const{
            static_assert(std::is_trivially_copyable<value_type>::value,
                          "Read Type must be trivially copy (plain old data only!)" );
            if (begin + length > end())
                throw std::out_of_range("offset + length can not exceed buffer size");
            value_type val;
            std::memcpy(&val, &(*begin), size);
            
            if (swap_bytes_ && length > 1)
                Swizzle(reinterpret_cast<char*>(&val), length);
            return val;
        };
        

        
        template<class container, ByteOrder order = ByteOrder::Default>
        container read(const_iterator begin, size_type count){
            using std::back_inserter;
            using std::copy;
            using std::reverse_copy;
            
            static_assert(std::is_trivially_copyable<typename container::value_type>::value,
                          "container must contain values that are trivially copyable. try creating a specialized read<T, ByteOrder> template to handle a container of this type" );
            container ret;
            size_type vsize = sizeof(typename container::value_type);
                        std::cout << "vsize: " << vsize << "\n";
            ret.reserve(count);
            auto end = begin + count * vsize;
            for (auto it = begin; it != end; it += vsize){
                ret.push_back(read<typename container::value_type, order>(it));
            }
            return ret;
        };
        
        
        template<class iterator_type>
        void write(iterator_type  begin, iterator_type end) {
            write(begin, end, this->begin());
        };
        
        template<class iterator_type>
        size_type write(iterator_type  begin, iterator_type end, iterator offset) {
            size_type bytes_written = std::min(std::distance(begin, end), std::distance(offset, this->end()));
            std::copy_n(begin, bytes_written, offset);
            return bytes_written;
        };
        
        template<class Type, ByteOrder order = ByteOrder::Default, bool swap_bytes_ = order != ByteOrder::Default>
        void write(Type data, iterator offset){
             static_assert(std::is_pointer<Type>::value,
                           "length is required for pointer values");
             static_assert(std::is_trivially_copyable<Type>::value,
                          "type of data must be trivially copyable (plain old data only!)" );
            std::memcpy(&(*offset), &data, sizeof(data));
            if (swap_bytes_ && sizeof(data) > 1) {
                Swizzle(reinterpret_cast<char*>(&(*offset)), sizeof(data));
            }
        };
        
        template<class Type, ByteOrder order = ByteOrder::Default, bool swap_bytes_ = order != ByteOrder::Default>
        void write(Type* data, iterator offset, size_type size){
            static_assert(std::is_trivially_copyable<Type>::value,
                          "type of data must be trivially copyable (plain old data only!)" );
            std::memcpy(&(*offset), data, size);
            if (swap_bytes_ && size > 1) {
                Swizzle(reinterpret_cast<char*>(&(*offset)), size);
            }
        }
        
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
        const size_t size() const;
        const size_type max_size() const;
        const bool empty() const;
        
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
        
    private:
        Buffer();
        uv_buf_t buffer_;
//        Encoding::Encoding encoding_;
        bool is_external_ = false;
        // required to cleanly dispose of the buffer
        std::shared_ptr<value_type> base_;
        
    };

                   

};
#endif
