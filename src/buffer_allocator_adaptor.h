//
//  buffer_allocator_adaptor.h
//  ngn
//
//  Created by Christopher Tarquini on 2/25/14.
//
//

#ifndef __ngn__buffer_allocator_adaptor__
#define __ngn__buffer_allocator_adaptor__
/*
#include <atomic>
#include <tq/type_traits.h>
#include "aligned_allocator.h"
namespace ngn { namespace detail {
    typedef uint8_t byte;
    
    enum buffer_flags : char {
        owned_buffer = 1,
        user_buffer = 2
    };
    
    struct buffer_header_base {
        buffer_header_base(bool is_owner, bool is_user, byte* buf, size_t size) :
        base(buf),
        size(size),
        ref_count(1),
        is_owner(is_owner),
        is_user_base(is_user) {};
        virtual void deallocate();
        
        byte * const base;
        size_t size;
        std::atomic_uint ref_count;
        bool is_owner;
        bool is_user_base;
    };
    template <class AllocT>
    struct buffer_header : buffer_header_base {
        using allocator_type = AllocT;
        buffer_header(byte* buffer,
                      size_t size,
                      bool is_owner = true,
                      bool is_user = false,
                      allocator_type alloc = allocator_type()) :
        buffer_header_base(is_owner, is_user, buffer, size),
        allocator(alloc) {};
        allocator_type allocator;
        
        virtual void deallocate() {
            ref_count.~atomic();
            if (is_user_base) {
                allocator.deallocate(base, size);
            } else {
                allocator.deallocate(this, size);
            }
        }
    };
    template <class T>
    class any_allocator {
        using value_type = T;
        using reference = tq::add_lvalue_reference_t<value_type>;
        using const_reference = tq::add_const_t<reference>;
        using pointer = tq::add_pointer_t<T>;
        using const_pointer = tq::add_const_t<pointer>;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using propagate_on_container_move_assignment = std::true_type;
        
        any_allocator() noexcept {};
        
        virtual pointer allocate(size_type n);
        virtual void deallocate(pointer p, size_type n);
        virtual void construct(pointer p, const_reference val);
        virtual size_type max_size() noexcept;
        virtual pointer address(reference x) const noexcept;
        virtual const_pointer address(const_reference x) const noexcept;
        template <class U, class... Args>
        void construct(U* p, Args&&... args) {
            ::new (reinterpret_cast<void*>(p)) U(std::forward<Args>(args)...);
        }
        template <class U>
        void destroy(U* p) {
            p->~U();
        }
        
    };
    class buffer_allocator_adaptor {
        using value_type = buffer_header_base;
        using pointer = value_type*;
        using const_pointer = const pointer;
        using size_type = std::size_t;
        
        template <class Alloc = std::allocator<char>>
        inline pointer allocate(size_t n) {
            
            using allocator_type = aligned_allocator_adaptor<Alloc, alignof(buffer_header<Alloc>)>;
            using header_type = buffer_header<allocator_type>;
            
            allocator_type allocator;
            static_assert(alignof(buffer_header<Alloc>) == alignof(header_type), "alignment mismtach");
            auto block = reinterpret_cast<header_type*>(allocator.allocate(sizeof(header_type) + n));
            auto header = new (reinterpret_cast<header_type*>(block)) header_type(reinterpret_cast<byte*>(block + sizeof(header_type)), true, false, allocator);
            
            
        }
        inline void deallocate(pointer p, size_type n) {
            if (p->is_owner &&
                p->ref_count.fetch_sub(1, std::memory_order_release) == 1) {
                std::atomic_thread_fence(std::memory_order_acquire);
                p->deallocate();
            } else if (p->is_owner) {
                p->deallocate();
                //delete[] reinterpret_cast<char*>(storage_->handle);
            }
        }
    };
}}*/

#endif /* defined(__ngn__buffer_allocator_adaptor__) */
