//
//  shared_memory_allocator.h
//  ngn
//
//  Created by Christopher Tarquini on 2/26/14.
//
//

#ifndef __ngn__shared_memory_allocator__
#define __ngn__shared_memory_allocator__

#include <memory>
#include <type_traits>
#include <cstddef>
#include <atomic>

#define REFERENCE_TYPES(T) \
    using reference = typename std::add_lvalue_reference<T>::type; \
    using const_reference = typename std::add_const<reference>::type; \

#define BASE_CONTAINER_TYPES(T) \
    using value_type = typename std::remove_reference<typename std::remove_const<T>::type>::type; \
    using pointer = typename std::add_pointer<value_type>::type; \
    using const_pointer = typename std::add_const<pointer>::type; \
    using size_type = std::size_t; \
    using difference_type = std::ptrdiff_t;

#define CONTAINER_TYPES(T) \
    BASE_CONTAINER_TYPES(T); \
    REFERENCE_TYPES(value_type);

#define VOID_CONTAINER_TYPES(T) \
    BASE_CONTAINER_TYPES(T);

namespace ngn { namespace detail {
    template <class T, class InnerAllocator = std::allocator<T>>
    class shared_memory_allocator {
        template <class OtherT>
        inline InnerAllocator rebind_inner(const OtherT& alloc) {
            using allocator_traits = typename std::allocator_traits<typename OtherT::inner_allocator_type>::template rebind_traits<T>;
            using rebound_type = typename allocator_traits::allocator_type;
            return rebound_type(alloc.m_alloc);
        }
        struct shared_header {
            using value_type = typename std::remove_const<typename std::remove_reference<T>::type>::type;
            std::atomic_uint ref_count;
            union {
                value_type value;
                char dummy;
            };
        };
    public:
        CONTAINER_TYPES(T);
        using inner_allocator_type = InnerAllocator;
        
        template< class U > struct rebind { typedef shared_memory_allocator<U, InnerAllocator> other; };
        
        shared_memory_allocator() : m_alloc(inner_allocator_type()) {};
        shared_memory_allocator(inner_allocator_type alloc) : m_alloc(alloc) {};
        
        template< class U >
        shared_memory_allocator( const shared_memory_allocator<U>& other ) :
            shared_memory_allocator(rebind_inner(other)) {};
        
        pointer allocate( size_type n, typename shared_memory_allocator<void, inner_allocator_type>::const_pointer hint = 0 ) {
            
        }
        void deallocate( pointer p, size_type n ) {

        }
        template< class U, class... Args >
        void construct( U* p, Args&&... args ) {
            std::allocator_traits<inner_allocator_type>::construct(m_alloc, p, std::forward<Args>(args)...);
        }
        template< class U >
        void destroy( U* p ) {
            p->~U();
        }
        bool acquire(pointer p) const {
            
        }
        size_type max_size() const noexcept {
            return m_alloc.max_size();
        };

        pointer address( reference x ) const {
            return &x;
        }
        const_pointer address(const_reference x ) const {
            return &x;
        }
        
    private:
        inner_allocator_type m_alloc;

    };
    
    template <class InnerAllocator>
    class shared_memory_allocator<void, InnerAllocator> {
        template <class OtherT>
        inline InnerAllocator rebind_inner(const OtherT& alloc) {
            using allocator_traits = typename std::allocator_traits<typename OtherT::inner_allocator_type>::template rebind_traits<void>;
            using rebound_type = typename allocator_traits::allocator_type;
            return rebound_type(alloc.m_alloc);
        }
    public:
        VOID_CONTAINER_TYPES(void);
        using inner_allocator_type = InnerAllocator;
        
        template< class U > struct rebind { typedef shared_memory_allocator<U, InnerAllocator> other; };
        
        shared_memory_allocator() : m_alloc(inner_allocator_type()) {};
        shared_memory_allocator(inner_allocator_type alloc) : m_alloc(alloc) {};
        
        template< class U >
        shared_memory_allocator( const shared_memory_allocator<U>& other ) :
        shared_memory_allocator(rebind_inner(other)) {};
        
    private:
        inner_allocator_type m_alloc;
        
    };
}}

#endif /* defined(__ngn__shared_memory_allocator__) */
