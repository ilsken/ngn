//
//  pointer_iterator.h
//  uv
//
//  Created by Christopher Tarquini on 10/15/13.
//  Copyright (c) 2013 Christopher Tarquini. All rights reserved.
//

#ifndef uv_pointer_iterator_h
#define uv_pointer_iterator_h
#include <iterator>
namespace ngn{
    using std::random_access_iterator_tag;
    using std::iterator;
template<typename TypeT>
class PointerIterator :
public std::iterator<random_access_iterator_tag, TypeT>
{
protected:
	TypeT* m_pData;
	
public:
	typedef random_access_iterator_tag iterator_category;
	typedef
    typename iterator<random_access_iterator_tag, TypeT>::value_type
    value_type;
	typedef
    typename iterator<random_access_iterator_tag, TypeT>::difference_type
    difference_type;
	typedef
    typename iterator<random_access_iterator_tag, TypeT>::reference
    reference;
	typedef
    typename iterator<random_access_iterator_tag, TypeT>::pointer
    pointer;
    
	PointerIterator() : m_pData(NULL) {}

	template<typename T2>
	PointerIterator(const PointerIterator<T2>& r) : m_pData(const_cast<typename std::remove_const<pointer>::type>(&(*r))) {}
	
	PointerIterator(pointer pData) : m_pData(pData) {}
    
	template<typename T2>
	PointerIterator& operator=(const PointerIterator<T2>& r)
    { m_pData = const_cast<typename std::remove_const<pointer>::type>(&(*r)); return *this; }
	
	PointerIterator& operator++()
    { ++m_pData; return *this; }
	
	PointerIterator& operator--()
    { --m_pData; return *this; }
    
	PointerIterator operator++(int)
    { return PointerIterator(m_pData++); }
	
	PointerIterator operator--(int)
    { return PointerIterator(m_pData--); }
    
	PointerIterator operator+(const difference_type& n) const
    { return PointerIterator(m_pData + n); }
    
	PointerIterator& operator+=(const difference_type& n)
    { m_pData += n; return *this; }
	
	PointerIterator operator-(const difference_type& n) const
    { return PointerIterator(pointer(m_pData - n)); }
    
	PointerIterator& operator-=(const difference_type& n)
    { m_pData -= n; return *this; }
	
	reference operator*() const
    { return *m_pData; }
    
	pointer operator->() const
    { return m_pData; }
    
	reference operator[](const difference_type& n) const
    { return m_pData[n]; }
    
	template<typename T>
	friend bool operator==(
                           const PointerIterator<T>& r1,
                           const PointerIterator<T>& r2);
    
	template<typename T>
	friend bool operator!=(
                           const PointerIterator<T>& r1,
                           const PointerIterator<T>& r2);
    
	template<typename T>
	friend bool operator<(
                          const PointerIterator<T>& r1,
                          const PointerIterator<T>& r2);
    
	template<typename T>
	friend bool operator>(
                          const PointerIterator<T>& r1,
                          const PointerIterator<T>& r2);
    
	template<typename T>
	friend bool operator<=(
                           const PointerIterator<T>& r1,
                           const PointerIterator<T>& r2);
    
	template<typename T>
	friend bool operator>=(
                           const PointerIterator<T>& r1,
                           const PointerIterator<T>& r2);
    
	template<typename T>
	friend typename PointerIterator<T>::difference_type operator+(
                                                                  const PointerIterator<T>& r1,
                                                                  const PointerIterator<T>& r2);
    
	template<typename T>
	friend typename PointerIterator<T>::difference_type operator-(
                                                                  const PointerIterator<T>& r1,
                                                                  const PointerIterator<T>& r2);
};

template<typename T>
bool operator==(const PointerIterator<T>& r1, const PointerIterator<T>& r2)
{ return (r1.m_pData == r2.m_pData); }

template<typename T>
bool operator!=(const PointerIterator<T>& r1, const PointerIterator<T>& r2)
{ return (r1.m_pData != r2.m_pData); }

template<typename T>
bool operator<(const PointerIterator<T>& r1, const PointerIterator<T>& r2)
{ return (r1.m_pData < r2.m_pData); }

template<typename T>
bool operator>(const PointerIterator<T>& r1, const PointerIterator<T>& r2)
{ return (r1.m_pData > r2.m_pData); }

template<typename T>
bool operator<=(const PointerIterator<T>& r1, const PointerIterator<T>& r2)
{ return (r1.m_pData <= r2.m_pData); }

template<typename T>	
bool operator>=(const PointerIterator<T>& r1, const PointerIterator<T>& r2)
{ return (r1.m_pData >= r2.m_pData); }

template<typename T>
typename PointerIterator<T>::difference_type operator+(
                                                       const PointerIterator<T>& r1,
                                                       const PointerIterator<T>& r2)
{ return PointerIterator<T>(r1.m_pData + r2.m_pData); }

template<typename T>
typename PointerIterator<T>::difference_type operator-(
                                                       const PointerIterator<T>& r1, const PointerIterator<T>& r2)
{ return (typename PointerIterator<T>::difference_type)((unsigned long)r1.m_pData - (unsigned long)r2.m_pData); }
}
#endif
