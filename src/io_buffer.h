//
//  io_buffer.h
//  ngn
//
//  Created by Christopher Tarquini on 2/17/14.
//
//

#ifndef __ngn__io_buffer__
#define __ngn__io_buffer__

#include <iostream>
#include <core/type_traits.hpp>
#include <scoped_allocator>
#include <atomic>
#include <new>
#include <assert.h>


namespace ngn { namespace experimental {
    static constexpr bool IS_BIG_ENDIAN = htonl(47) == 47;
    enum ByteOrder {
        Default = BYTE_ORDER,
        LittleEndian = LITTLE_ENDIAN,
        BigEndian = BIG_ENDIAN
    };
    
    
    
    
    
    typedef uint8_t byte;
    class Buffer {
    public:
        static struct take_ownership_tag {} take_ownership;
        static struct non_owning_tag {} non_owning;
        static struct slice_buffer_tag {} slice_buffer;

        // thou shalt not initialize on the stack
        struct buffer_storage_base {
            virtual void aquire() = 0;
            virtual void release() = 0;
        };
      
        template <class AllocT>
        struct inline_buffer_storage : buffer_storage_base {
          using allocator_type = AllocT;
          using allocator_traits = std::allocator_traits<allocator_type>;
          using pointer = typename  allocator_traits::pointer;
          inline_buffer_storage(byte* buf, size_t size, const allocator_type& alloc) : base(buf), size(size), allocator(alloc)
          {
            use_count.exchange(1, std::memory_order_relaxed);
          };
          virtual void aquire() {
            use_count.fetch_add(1u, std::memory_order_relaxed);
          }
          virtual void release()
          {
            if (use_count.fetch_sub(1u, std::memory_order_release) == 1)
            {
              std::atomic_thread_fence(std::memory_order_acquire);
              auto block = reinterpret_cast<byte*>(this);
              auto block_size = size;
              allocator_type alloc { std::move(allocator) };
              // this is wierd and gross
              this->~inline_buffer_storage();
              allocator_traits::deallocate(alloc, block, block_size);
            }
          }
          byte* base;
          size_t size;
          std::atomic_uint use_count;
          allocator_type allocator;
        };
      
        template <class AllocT>
        buffer_storage_base* make_buffer_storage(const AllocT& allocator, size_t size, byte*& base) {
           using traits = std::allocator_traits<AllocT>;
           using allocator_t = AllocT;
           using storage_t = inline_buffer_storage<AllocT>;
           using std::align;
          
           const size_t padding = sizeof(storage_t) + alignof(storage_t) - 1;
           const size_t total_size = size + padding;
           byte* block;
           void* storage_ptr;
           allocator_t alloc { allocator };
           size_t aligned_space = padding;
          
           storage_ptr = block = traits::allocate(alloc, total_size, nullptr);
           // make the storage pointer aligned properly
           if (align(alignof(storage_t), sizeof(storage_t), storage_ptr, aligned_space) != nullptr)
           {
             new (reinterpret_cast<storage_t*>(storage_ptr)) storage_t(base, total_size, allocator);
             base = reinterpret_cast<byte*>(storage_ptr) + sizeof(storage_t);
             return reinterpret_cast<storage_t*>(storage_ptr);
           }
           else
           {
             throw std::bad_alloc {};
           }
           return nullptr;
        }

        template <class AllocT, class T>
        using rebind_allocator_t = typename std::allocator_traits<AllocT>::template rebind_traits<T>::allocator_type;
      
        class buffer_control_block_base {
          virtual void aquire() = 0;
          virtual void release() = 0;
          virtual byte* data();
          virtual size_t size();
        };
      
        template<class T, class Allocator = std::allocator<T>, class ReferenceCount = void>
        class buffer_control_block;
      
        template<class T, class Allocator>
        class buffer_control_block<T, Allocator, void> {
          virtual void aquire() {};
          virtual void release() {};
          private:
            
        };
  /*
        template <class AllocT>
        class scoped_buffer_allocator_adaptor;
      
        template <class AllocT>
        using scoped_buffer_allocator_adaptor_base = std::scoped_allocator_adaptor<rebind_allocator_t<AllocT,
                               buffer_header<scoped_buffer_allocator_adaptor<AllocT>>
                              >,
            AllocT
            >;
      
      
        template <class AllocT>
        class scoped_buffer_allocator_adaptor
          : public scoped_buffer_allocator_adaptor_base<AllocT> {
          public:
            
            //using scoped_buffer_allocator_adaptor<wrap_user_buffer_allocator<AllocT>, AllocT>::scoped_buffer_allocator_adaptor;
            template <class U, class... Args>
            inline void construct (U* p, bool is_owner, Args&&... args) {
                new (p) U(is_owner, true, *this, std::forward<Args>(args)...);
            }
            template <class U>
            inline void destroy (U* p) {
                assert(p->is_user_base);
                if (p->is_owner)
                  this->inner_allocator().deallocate(p->base, p->size);
                p->~U();
            }
          
            
        };*/

        
    public:
        //
        // Iterator Traits
        //
        using iterator_traits = std::iterator_traits<byte*>;
        using const_iterator_traits = std::iterator_traits<const byte*>;
        
        //
        // Value Types
        //
        using value_type        = iterator_traits::value_type;
        using reference         = iterator_traits::reference;
        using pointer           = iterator_traits::pointer;
        using const_reference   = const_iterator_traits::reference;
        using const_pointer     = const_iterator_traits::pointer;
        
        //
        // Iterator Types
        //
        using iterator                  = pointer;
        using const_iterator            = const_pointer;
        using reverse_iterator          = std::reverse_iterator<iterator>;
        using const_reverse_iterator    = std::reverse_iterator<const_iterator>;
        
        
        //
        // Size/Difference Types
        //
        using size_type         = size_t;
        using difference_type   = iterator_traits::difference_type;
        
        template <class AllocT = std::allocator<byte>>
        explicit Buffer(size_type size,
                        AllocT alloc = AllocT()) :
        size_(size) {
          storage_ = make_buffer_storage(alloc, size, data_);
        }
        
        // copy constructors
        Buffer(const_iterator begin, const_iterator end);
        Buffer(const_iterator begin, size_type size);
        
        template <class iterator>
        Buffer(iterator begin, iterator end)
        : Buffer(std::distance(begin, end)) {
            std::copy(begin, end, data_);
        }
        
        
        
        
        // Take Ownership
        Buffer(std::unique_ptr<value_type[]>&& source, size_type offset, size_type size);
        
        
        // copy constructor
        Buffer(const Buffer& other) noexcept;
        // move constructor
        Buffer(Buffer&& other) noexcept;
        
        
        
        
        
        ~Buffer() noexcept;
        
        //
        // Modifiers
        //
        void swap(Buffer& other);
        
        //
        // Views
        //
        Buffer slice(iterator begin, iterator end);
        
        //
        // Smart Pointer Methods
        //
        uint use_count() const noexcept;
        bool is_owned() const noexcept;
        uint ref() noexcept;
        
        //
        // Container Methods
        //
        
        // Capacity
        size_type size() const noexcept;
        size_type max_size() const noexcept;
        bool empty() const noexcept;
        
        
        // Element Access
        reference at(size_type index);
        const_reference at(size_type index) const;
        reference front();
        const_reference front() const;
        
        pointer data();
        const_pointer data() const;
        
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
        
        //
        // Operators
        //
        bool operator==(const Buffer& rhs) const;
        bool operator!=(const Buffer& rhs) const;
        Buffer& operator=(Buffer other);
        const_reference operator[](size_t index) const;
        reference operator[](size_t index);
        operator bool() const noexcept;
        
        //private:
    private:
        friend constexpr size_t managed_buffer_space();
        static inline size_t managed_buffer_size(size_t capacity);
//        Buffer(buffer_header* header, iterator begin, iterator end, slice_buffer_tag);
        Buffer(pointer buffer, size_type offset, size_type size, take_ownership_tag);
        Buffer(pointer buffer, size_type offset, size_type size, non_owning_tag);
        buffer_storage_base* storage_;
        pointer data_;
        size_type size_;
        
    };
    
} }
#endif /* defined(__ngn__io_buffer__) */
