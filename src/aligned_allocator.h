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
namespace ngn { namespace detail{
    struct empty_t {
    };

    template <class Alloc, class T>
    using rebind_t = typename std::allocator_traits<Alloc>::template rebind_traits<T>::allocator_type;
    template <typename Alloc,
    size_t Align = std::alignment_of<typename std::allocator_traits<Alloc>::value_type>::value_type>
    class aligned_allocator_adaptor : private rebind_t<Alloc, unsigned char> {
        using alignment_header = ptrdiff_t;
        static const size_t padding_size = sizeof(alignment_header) + (Align > alignof(alignment_header) ? Align : alignof(alignment_header)) - 1;
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
            uint32_t magic = 0xDEADBABE;
            const size_type total_size = (sizeof(value_type) * (n - 1)) + padding_size;
            // use char allocator for internal storage
            const size_t alignment = alignof(alignment_header);
            auto block = block_allocator_traits::allocate(static_cast<block_allocator_type&>(*this), total_size, hint);
          
            auto aligned_block = reinterpret_cast<char*>(reinterpret_cast<size_t>(block + (alignment - 1)) & -alignment);
            if (static_cast<size_t>(aligned_block - block) <= padding_size - sizeof(alignment_header))
            {
                block_allocator_traits::deallocate(static_cast<block_allocator_type&>(*this),
                                                   reinterpret_cast<block_pointer>(block),
                                                    total_size);
                throw std::bad_alloc();
            }
          
            //void* magic_ptr = (char*)block + total_size - sizeof(magic) - 1;
            //memcpy(magic_ptr, (void*)&magic, sizeof(magic));
            *reinterpret_cast<alignment_header*>(aligned_block) = 
            aligned_block += sizeof(alignment_header);
          
            // offset must be correct
            assert(aligned_block + *aligned_block == block);
            // make sure offsetof stuff works so we can get the header from teh base
            assert(reinterpret_cast<block_pointer>(&aligned_block->base) - offsetof(alignment_header, base) == reinterpret_cast<block_pointer>(aligned_block));
            // make sure we allocated enough space for n elements after the header
            assert((reinterpret_cast<block_pointer>(block) + total_size - reinterpret_cast<block_pointer>(&aligned_block->base)) / sizeof(value_type) >= n);
            // make sure everything's aligned correctly
            assert((uintptr_t)aligned_block % alignof(alignment_header) == 0);
            assert((uintptr_t)&aligned_block->base % alignof(value_type) == 0);
            std::cout << "allocated " << std::dec << total_size << " bytes at " << std::hex << block <<  " with capacity " << std:: dec << n << std::endl;
            return reinterpret_cast<pointer>(&aligned_block->base);
        }
        inline void deallocate(pointer p, size_type n) {
            // base pointer - base offset = header pointer
            auto header = reinterpret_cast<alignment_header*>(reinterpret_cast<block_pointer>(p) - offsetof(alignment_header, base));
            auto block = reinterpret_cast<block_pointer>(header) - header->offset;
            const size_type total_size = (sizeof(value_type) * (n - 1)) + aligned_header_size + sizeof(uint32_t);
            //0xDEADBABE
            uint32_t magic = 0;
            memcpy(&magic, block + total_size - sizeof(magic) - 1, sizeof(magic));
            assert(magic == 0xDEADBABE);
            block_allocator_traits::deallocate(static_cast<block_allocator_type&>(*this),
                                               block,
                                               total_size);
        }
        template <class U, class... Args>
        inline void construct (U* p, Args&&... args) {
            allocator_traits::construct(static_cast<allocator_type&>(*this), p, std::forward<Args>(args)...);
        }
        template <class U>
        inline void destroy (U* p) {
            allocator_traits::destroy(static_cast<allocator_traits&>(*this), p);
        }
    }; // class aligned_allocator
}}

#endif /* defined(__tq__aligned_allocator__) */
