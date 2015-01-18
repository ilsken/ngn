//
//  io_buffer.h
//  ngn
//
//  Created by Christopher Tarquini on 2/17/14.
//
//

#ifndef __ngn__io_buffer__
#define __ngn__io_buffer__

#include "pointer_iterator.h"
#include "transform_traits.h"

#include <iostream>
#include <core/type_traits.hpp>
#include <scoped_allocator>
#include <atomic>
#include <new>
#include <assert.h>

#define NGN_ENABLE_IF(R) class = tq::enable_if_t<R::value>


namespace ngn { namespace experimental {
    static constexpr bool IS_BIG_ENDIAN = htonl(47) == 47;
    enum ByteOrder {
        Default = BYTE_ORDER,
        LittleEndian = LITTLE_ENDIAN,
        BigEndian = BIG_ENDIAN
    };
    // detects containers with a data() member
    // in STL all of these have contigous storage
   /* template <class T>
    class is_contiguous_helper {
    public:
        template <class C>
        static tq::is_iterator_category_t<typename C::iterator, std::random_access_iterator_tag > check_tag(int);
        
        template <class C>
        static std::false_type check_tag(...);
        
        template<class C>
        static tq::is_same_t<
        tq::decay_t<typename C::value_type>,
        tq::decay_t<tq::remove_pointer_t<decltype(std::declval<C>().data())>>> check(int);
        template <class C>
        static std::false_type check(...);
    public:
        typedef tq::conditional_t<
        decltype(check_tag<T>(0))::value && decltype(check<T>(0))::value,
        std::true_type,
        std::false_type>  result;
    };*/
    
    /*template <class T> struct is_contiguous_container : is_contiguous_helper<T>::result {};*/
    
    /*template <class T>
    class is_trivial_helper {
        template <class C>
        static detail::is_trivially_copyable_t<typename C::value_type> check(int);
        //static typename std::is_trivially_copyable<typename C::value_type>::type check(int);
        template <class C>
        static std::false_type check(...);
    public:
        typedef decltype(check<T>(0)) result;
    };
    
    template <class T> struct is_trivial_container : is_trivial_helper<T>::result {};
    
    template <class T> struct is_contiguous_trivial_container
    : std::integral_constant<bool, is_contiguous_container<T>::value && is_trivial_container<T>::value> {};
    
    */
    
    
    
    
    
    typedef uint8_t byte;
    class Buffer {
    public:
        static struct take_ownership_tag {} take_ownership;
        static struct non_owning_tag {} non_owning;
        static struct slice_buffer_tag {} slice_buffer;

        struct buffer_header_base2 {
            std::size_t size;
            std::atomic_uint ref_count;
            virtual void release() = 0;
        };
        template <class Alloc> struct buffer_header2 : buffer_header_base2 {
            Alloc allocator;
            virtual void release() {
                
            }
        };
        // thou shalt not initialize on the stack
        struct buffer_header_base {
            buffer_header_base(byte* buf, size_t size) :
            base(buf),
            size(size) {
            };
            
            byte * const base;
            size_t size;
            virtual void aquire() = 0;
            virtual void release() = 0;
        };
      
        template <class AllocT>
        struct buffer_inline_storage : buffer_header_base {
          using allocator_type = AllocT;
          using allocator_traits = std::allocator_traits<allocator_type>;
          using pointer = typename  allocator_traits::pointer;
          buffer_inline_storage(byte* buf, size_t size, allocator_type alloc) : buffer_header_base(buf, size), allocator(alloc) {
          };
          
          virtual void release()
          {
            if (use_count.fetch_sub(1, std::memory_order_release) == 1) {
              std::atomic_thread_fence(std::memory_order_acquire);
              auto buffer_size = size;
              ~buffer_inline_storage();
              allocator_traits::destroy(allocator, this);
              allocator_traits::deallocate(allocator,this, buffer_size);
            }
          }
          std::atomic_uint use_count;
          allocator_type allocator;
        };
        /*template <class AllocT>
        struct buffer_header : buffer_header_base {
            using allocator_type = AllocT;
            using allocator_traits = std::allocator_traits<allocator_type>;
            buffer_header(bool is_owner,
                          bool is_user,
                          allocator_type alloc, byte* base, size_t size) :
            buffer_header_base(is_owner, is_user, base, size),
            allocator(alloc) {};
            virtual void release() {
                auto buffer_size = size;
                allocator_traits::destroy(allocator, this);
                allocator_traits::deallocate(allocator,this, buffer_size);
            }
            AllocT allocator;
        };*/
    
    template <class T, class Deleter = std::default_delete<T>>
    class deleter_allocator_wrapper : public std::allocator<T> {
    public:
      using traits = std::allocator_traits<std::allocator<T>>;
      using pointer = typename traits::pointer;
      using size_type = typename traits::size_type;
      deleter_allocator_wrapper(Deleter deleter) : m_deleter(deleter){
      }
      void deallocate(pointer p, size_type n) {
        m_deleter(p);
      }
      private:
        Deleter m_deleter;
    };
        template <class AllocT, class T>
        using rebind_allocator_t = typename std::allocator_traits<AllocT>::template rebind_traits<T>::allocator_type;
        // good fucking lord what were you thinking?
        // jesus christ man clean this shit up
        // there's just no reason for this hackery to be here
        template <class AllocT>
        class buffer_allocator_adaptor :  rebind_allocator_t<AllocT, byte> {
            using block_allocator_type = rebind_allocator_t<AllocT, byte>;
            using block_allocator_traits = std::allocator_traits<block_allocator_type>;
        public:
            using wrapped_allocator_type = AllocT;
            using value_type = buffer_header<buffer_allocator_adaptor>;
            using pointer = core::add_pointer_t<value_type>;
            using const_pointer = core::add_const_t<pointer>;
            using reference = core::add_lvalue_reference_t<value_type>;
            using const_reference = core::add_const_t<reference>;
            using size_type = typename block_allocator_traits::size_type;
            using difference_type = typename block_allocator_traits::difference_type;
            static constexpr size_type padding_size = sizeof(value_type) + alignof(value_type) - 1;
            buffer_allocator_adaptor() = default;
            template< class U >
            buffer_allocator_adaptor( const buffer_allocator_adaptor<U>& other ) : block_allocator_type(static_cast<const typename buffer_allocator_adaptor<U>::block_allocator_type&>(other)) {};
            buffer_allocator_adaptor(const buffer_allocator_adaptor& other) : block_allocator_type(static_cast<const block_allocator_type&>(other)) {};
            buffer_allocator_adaptor(const wrapped_allocator_type& other) : block_allocator_type(other) {};
            inline pointer allocate(size_type n, const void* hint = 0) {
                size_type total_size = n + padding_size, header_space = padding_size;
                auto block = block_allocator_type::allocate(total_size, hint);
                auto aligned_block = reinterpret_cast<pointer>(std::align(alignof(value_type), sizeof(value_type), block, header_space));
                if (aligned_block != nullptr)
                {
                  
                } else
                    throw std::bad_alloc();
              
                std::cout << "buffer allocated " << std::dec << total_size << " bytes at " << std::hex << ptr << " with capacity: " << std::dec << n << "\n";
                std::cout << "buffer_header: size = " << sizeof(value_type) << ", align = " << alignof(value_type) << std::endl;
                std::cout << "byte*: " << alignof(byte*) << std::endl;
                std::cout << "size_t*: " << alignof(size_type) << std::endl;
                std::cout << "atomic: " << alignof(std::atomic_uint) << std::endl;
                std::cout << "alloc: " << alignof(AllocT) << std::endl;
              
                return ptr;
            }
            inline void deallocate(pointer p, size_type n) {
                std::cout << "deallocating pointer: " << std::hex << p << " with capacity: " << std::dec <<  n << "\n";
                aligned_allocator_type::deallocate(reinterpret_cast<typename aligned_allocator_type::pointer>(p), sizeof(value_type) + n);
            }
            template <class U, class... Args>
            inline void construct (U* p, Args&&... args) {
                new (p) U(reinterpret_cast<byte*>(p) + sizeof(value_type), std::forward<Args>(args)..., *this);
            }
            template <class U>
            inline void destroy (U* p) {
                p->~U();
            }
            
        };
  
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
          
            
        };
        template <class AllocT>
        static buffer_header_base* make_storage(size_t capacity, AllocT alloc) {
            using allocator_type = wrap_buffer_allocator<AllocT>;
            allocator_type allocator(alloc);
            auto header = allocator.allocate(capacity);
            allocator.construct(header, capacity);

          
            return header;
        }
        /*template <class AllocT>
        static buffer_header_base* make_storage(bool is_owner, byte* base, size_t capacity, AllocT alloc) {
            using allocator_type = scoped_buffer_allocator_adaptor<AllocT>;
            using outer_allocator = typename allocator_type::outer_allocator_type;
            using inner_allocator = typename allocator_type::inner_allocator_type;
            using allocator_traits = std::allocator_traits<allocator_type>;
            outer_allocator outer{};
            const inner_allocator& inner = alloc;
            inner_allocator x { inner };
            outer_allocator y { outer_allocator{}};
            std::scoped_allocator_adaptor<rebind_allocator_t<AllocT, buffer_header<scoped_buffer_allocator_adaptor<AllocT>>>, AllocT> wtf { rebind_allocator_t<AllocT, buffer_header<scoped_buffer_allocator_adaptor<AllocT>>>{}, alloc  };
          
            allocator_type lol { outer_allocator{} };
            /*auto header = allocator.allocate(1);
            allocator_traits::construct(header, is_owner, base, capacity);
            allocator.construct(header, is_owner, base, capacity);
            return header;
            return nullptr;
        };*/
        
        /*
         template <class AllocT>
         static buffer_header_base* make_storage(size_t capacity, AllocT alloc)  {
         using allocator_type = detail::aligned_allocator_adaptor<AllocT, alignof(buffer_header<std::scoped_allocator_adaptor<AllocT, AllocT>>)>;
         using scoped_allocator_type = std::scoped_allocator_adaptor<allocator_type, AllocT>;
         using header_type = buffer_header<scoped_allocator_type>;
         
         allocator_type allocator(alloc);
         scoped_allocator_type scoped_allocator(allocator, alloc);
         static_assert(alignof(buffer_header<std::scoped_allocator_adaptor<AllocT, AllocT>>) == alignof(header_type), "alignment mismtach");
         auto block = reinterpret_cast<header_type*>(allocator.allocate(sizeof(header_type) + capacity));
         auto header = new (reinterpret_cast<header_type*>(block)) header_type(reinterpret_cast<byte*>(block + sizeof(header_type)), true, false, scoped_allocator);
         
         return header;
         }*/
        
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
        explicit Buffer(size_type capacity,
                        AllocT alloc = AllocT()) :
        storage_(make_storage(capacity, alloc)),
        data_(storage_->base),
        size_(storage_->size) {
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
        buffer_header_base* storage_;
        pointer data_;
        size_type size_;
        
    };
    
} }
#endif /* defined(__ngn__io_buffer__) */
