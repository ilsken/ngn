# Buffer(StorageFlags, char* buffer, char* base,char* data, size_t size, deleter)
# Buffer(unique_ptr<char&&>, offset, length)
# Buffer(const std::string&, offset, length)
# Buffer(const char*, offset, length);
# Buffer::TakeOwnership(char*, offset, length, deleter);
# Buffer(const_iterator begin, const_iterator_end);
# Buffer::Unsafe(char* ptr, offset, length);
# read<T>(offset, prevent_overflow)
# write<T>(offset, prevent_overflow)
# slice(const_iterator begin, const_iterator end)
# length()
# size()
# begin()/cbegin()
# end()/cend()
# front()/back()
# at(ptrdiff_t)
# max_size()
# fill(value, begin, end)
# concat(begin, end, sizeHint)

# StorageFlags
- Owned (Buffer owns the data)
- Shared (data is shared with other Buffers)
- UserOwned (user owns the data, user must keep it alive)
# BufferStorage

