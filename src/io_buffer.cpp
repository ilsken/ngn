//
//  io_buffer.cpp
//  ngn
//
//  Created by Christopher Tarquini on 2/17/14.
//
//

#include "io_buffer.h"
#include <assert.h>


namespace {
    
}
namespace ngn { namespace experimental {
    typedef Buffer::iterator iterator;
    typedef Buffer::const_iterator const_iterator;
    typedef Buffer::reference reference;
    typedef Buffer::const_reference const_reference;
    typedef Buffer::value_type value_type;
    typedef Buffer::difference_type difference_type;
    typedef Buffer::reverse_iterator reverse_iterator;
    typedef Buffer::const_reverse_iterator const_reverse_iterator;
    typedef Buffer::size_type size_type;
    typedef Buffer::difference_type difference_type;
    typedef Buffer::pointer pointer;
    typedef Buffer::const_pointer const_pointer;
    
    


    



    
    /*
    Buffer::Buffer(std::unique_ptr<value_type>&& source, size_type offset, size_type size)
    : Buffer(source.release(), offset, size, take_ownership) {};
    
    Buffer::Buffer(pointer buffer, size_type offset,
                   size_type size, take_ownership_tag)
    : storage_(new BufferHeader(buffer, true)),
    data_(buffer+offset),
    size_(size){};
    
    Buffer::Buffer(pointer buffer, size_type offset, size_type size, non_owning_tag)
    : storage_(new BufferHeader(buffer)),
      data_(buffer+offset),
      size_(size) {};*/
    /*
     Buffer::Buffer(const Buffer& other, size_type offset, size_type size) : storage_(other.storage_), data_(other.data_ + offset), size_(size) {
     if (storage_->is_owned) storage_->ref_count.fetch_add(1, std::memory_order_relaxed);
     }*/
    
    // optimize for buffers
    Buffer::Buffer(const_iterator begin, const_iterator end)
    : Buffer(std::distance(begin, end)) {
        std::memcpy(data_, begin, size_);
    }
    // copy constructor
    Buffer::Buffer(const Buffer& other) noexcept {
        // should we copy the buffer?
        // copying the data with  memcpy would be more like string
        // but it makes it confusing to work with
        if (*this != other) {
            storage_ = other.storage_;
            size_ = other.size_;
            data_ = other.data_;
            if (other && storage_->is_owner) {
                storage_->ref_count.fetch_add(1, std::memory_order_relaxed);
            }
        }
    };
    // move constructor
    Buffer::Buffer(Buffer&& other) noexcept : data_(other.data_), size_(other.size_) {
        using std::swap;
        // other buffer destructor will take care of ref counts
        swap(storage_, other.storage_);
    }
    
    // slice constructor
    /*Buffer::Buffer(buffer_header_base* header, iterator begin, iterator end, slice_buffer_tag)
    : storage_(header),
    data_(begin),
    size_(std::distance(begin, end)){
        storage_->ref_count.fetch_add(1, std::memory_order_relaxed);
    }*/
    
    Buffer::~Buffer() noexcept {
        // if we are the last owner of a user-defined buffer, free it's memory
        if (storage_->is_owner &&
            storage_->ref_count.fetch_sub(1, std::memory_order_release) == 1) {
            std::atomic_thread_fence(std::memory_order_acquire);
            storage_->release();
        } else if (!storage_->is_owner) {
            storage_->release();
            //delete[] reinterpret_cast<char*>(storage_->handle);
        }
    };
    
    //
    // Modifiers
    //
    void Buffer::swap(Buffer& other) {
        using std::swap;
        swap(size_, other.size_);
        swap(data_, other.data_);
        swap(storage_, other.storage_);
    };
    
    void Buffer::fill(const_reference value) {
        fill(begin(), end(), value);
    };
    void Buffer::fill(iterator begin, iterator end, const_reference value) {
        std::memset(begin, value, std::distance(begin, end));
    };
    void Buffer::fill_n(const_reference value, iterator begin, size_type count) {
        std::memset(begin, value, count);
    };
    
    //
    // Views
    //
    /*Buffer Buffer::slice(iterator begin, iterator end) {
        return Buffer(this->storage_, begin, end, slice_buffer);
    };
    
    //
    // Smart Pointer Methods
    //
    uint Buffer::use_count() const noexcept {
        return storage_->ref_count.load();
    };
    bool Buffer::is_owned() const noexcept {
        return storage_->is_owned;
    };
    
    //
    // Container Methods
    //
    size_type Buffer::size() const noexcept {
        return size_;
    };
    size_type Buffer::max_size() const noexcept {
        return size_;
    }
    bool Buffer::empty() const noexcept {
        return size_ == 0 || storage_->is_owned ? false : storage_->user_base == nullptr;
    };*/
    
    //
    // Element Access
    //
    reference Buffer::at(size_type index) {
        if (index < size_) return (*this)[index];
        else throw std::out_of_range("index must be less than size of buffer");
    };
    const_reference Buffer::at(size_type index) const {
        if (index < size_) return (*this)[index];
        else throw std::out_of_range("index must be less than size of buffer");
    };
    reference Buffer::front() {
        return data_[0];
    };
    const_reference Buffer::front() const {
        return data_[0];
    };
    pointer Buffer::data() {
        return data_;
    };
    const_pointer Buffer::data() const {
        return data_;
    };
    
    //
    // Iterators
    //
    const_iterator Buffer::cbegin() const {
        return Buffer::const_iterator(&(*this)[0]);
    };
    const_iterator Buffer::cend() const {
        return Buffer::const_iterator(&(*this)[size_]);
    };
    const_reverse_iterator Buffer::crbegin() const {
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
        return Buffer::iterator(&(*this)[size_]);
    };
    Buffer::const_iterator Buffer::end() const{
        return cend();
    };
    Buffer::reverse_iterator Buffer::rbegin() {
        return Buffer::reverse_iterator(end());
    };
    Buffer::reverse_iterator Buffer::rend() {
        return Buffer::reverse_iterator(begin());
    }
    
    
    //
    // Operators
    //
    
    // copy-assignment
    Buffer& Buffer::operator=(Buffer other) {
        //using std::swap;
        this->swap(other);
        return *this;
    };
    
    bool Buffer::operator==(const Buffer& other) const{
        return data_ == other.data_ && size_ == other.size_;
    };
    bool Buffer::operator!=(const Buffer& other) const{
        return !(*this == other);
    };
    
    const_reference Buffer::operator[](size_t index) const{
        return data_[index];
    };
    
    reference Buffer::operator[](size_type index){
        return data_[index];
    };
    
    Buffer::operator bool() const noexcept {
        return data_ != nullptr;
    };
}} // namespace



