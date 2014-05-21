//
//  aligned_allocator.h
//  tq
//
//  Created by Christopher Tarquini on 2/25/14.
//
//

#ifndef __tq__aligned_allocator__
#define __tq__aligned_allocator__

#include <cstddef>
#include <memory>
#include <assert.h>
#include <tq/type_traits.h>

namespace ngn { namespace detail{
    struct empty_t {
    };
    template <class Alloc, class T>
    using rebind_t = typename std::allocator_traits<Alloc>::template rebind_traits<T>::allocator_type;
    template <typename Alloc,
    size_t Align = std::alignment_of<typename std::allocator_traits<Alloc>::value_type>::value>
    class aligned_allocator_adaptor : private rebind_t<Alloc, unsigned char> {
        struct alignment_header;
        static const size_t header_size = sizeof(alignment_header) + Align - 1;
        using allocator_traits = std::allocator_traits<Alloc>;
        using allocator_type = typename allocator_traits::allocator_type;
        using block_allocator_type = rebind_t<Alloc, unsigned char>;
        using block_allocator_traits = std::allocator_traits<block_allocator_type>;
        using block_pointer = typename block_allocator_traits::pointer;
    public:

        using pointer = typename allocator_traits::pointer;
        using const_pointer = typename allocator_traits::const_pointer;
        using value_type = typename allocator_traits::value_type;
        using size_type = typename allocator_traits::size_type;
        
 
        
        template< class U >
        aligned_allocator_adaptor( const aligned_allocator_adaptor<U>& other ) : aligned_allocator_adaptor(static_cast<const allocator_type&>(other)) {}
        
        aligned_allocator_adaptor(const aligned_allocator_adaptor& other) : aligned_allocator_adaptor(static_cast<const allocator_type&>(other)){};
        

        aligned_allocator_adaptor(const allocator_type& other) :
            allocator_type(other) {};
        
                                
        inline pointer allocate(size_type n, const void* hint = 0) {
            assert(n > 0);
            const size_type total_size = (sizeof(value_type) * (n - 1)) + header_size;
            // use char allocator for internal storage
            void* block = block_allocator_traits::allocate(static_cast<block_allocator_type&>(*this), total_size, hint);
            size_t new_header_size = header_size;
            auto aligned_block = reinterpret_cast<alignment_header*>(std::align(Align, header_size, block, new_header_size));
            if (!aligned_block) {
                block_allocator_traits::deallocate(static_cast<block_allocator_type&>(*this),
                                                   reinterpret_cast<block_pointer>(block),
                                                   total_size);
                throw std::bad_alloc();
            }
            
            new (aligned_block) alignment_header(reinterpret_cast<block_pointer>(aligned_block) - reinterpret_cast<block_pointer>(block));
            assert(aligned_block + aligned_block->offset == block);
            assert(reinterpret_cast<block_pointer>(&aligned_block->base) - offsetof(alignment_header, base) == reinterpret_cast<block_pointer>(aligned_block));
            assert((reinterpret_cast<block_pointer>(block) + total_size - reinterpret_cast<block_pointer>(&aligned_block->base)) / sizeof(value_type) >= n);
       
            return reinterpret_cast<pointer>(&aligned_block->base);
        }
        inline void deallocate(pointer p, size_type n) {
            auto header = reinterpret_cast<alignment_header*>(reinterpret_cast<block_pointer>(p) - offsetof(alignment_header, base));
            block_allocator_traits::deallocate(static_cast<block_allocator_type&>(*this), header->handle(), (sizeof(value_type) * (n - 1)) + header_size);
        }
        template <class U, class... Args>
        inline void construct (U* p, Args&&... args) {
            allocator_traits::construct(static_cast<allocator_type&>(*this), p, std::forward<Args>(args)...);
        }
        template <class U>
        inline void destroy (U* p) {
            allocator_traits::destroy(static_cast<allocator_traits&>(*this), p);
        }
        
    private:
        struct alignment_header {
            using allocator_traits = std::allocator_traits<Alloc>;
            using pointer = typename allocator_traits::pointer;
            using value_type = typename allocator_traits::value_type;
            
            using storage_type = std::aligned_storage<sizeof(value_type), Align>;
            inline pointer handle() {
                return reinterpret_cast<pointer>(this) - offset;
            };
            alignment_header (ptrdiff_t offset) : offset(offset) {};
            ptrdiff_t offset;
            storage_type base;
        };
    }; // class aligned_allocator
}}

#endif /* defined(__tq__aligned_allocator__) */
