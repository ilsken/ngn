// ngn::any
// Standalone version of boost::any

// See http://www.boost.org/libs/any for Documentation.

#ifndef NGN_ANY_INCLUDED
#define NGN_ANY_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// what:  variant type boost::any
// who:   contributed by Kevlin Henney,
//        with features contributed and bugs found by
//        Antony Polukhin, Ed Brey, Mark Rodgers,
//        Peter Dimov, and James Curran
// when:  July 2001, Aplril 2013

#include <algorithm>
#include <type_traits>
#include <typeinfo>



// See boost/python/type_id.hpp
// TODO: add NGN_TYPEID_COMPARE_BY_NAME to config.hpp
# if (defined(__GNUC__) && __GNUC__ >= 3) \
|| defined(_AIX) \
|| (   defined(__sgi) && defined(__host_mips)) \
|| (defined(__hpux) && defined(__HP_aCC)) \
|| (defined(linux) && defined(__INTEL_COMPILER) && defined(__ICC))
#  define NGN_AUX_ANY_TYPE_ID_NAME
#include <cstring>
# endif

namespace ngn
{
    using std::remove_reference;
    using std::is_reference;
    
    class any
    {
    public: // structors
        
        any() noexcept
        : content(0)
        {
        }
        
        template<typename ValueType>
        any(const ValueType & value)
        : content(new holder<ValueType>(value))
        {
        }
        
        any(const any & other)
        : content(other.content ? other.content->clone() : 0)
        {
        }
        
#ifndef NGN_NO_CXX11_RVALUE_REFERENCES
        // Move constructor
        any(any&& other) noexcept
        : content(other.content)
        {
            other.content = 0;
        }
        
        // Perfect forwarding of ValueType
        template<typename ValueType>
        any(ValueType&& value, typename std::enable_if<std::is_same<any&, ValueType>::value>)
        : content(new holder< typename remove_reference<ValueType>::type >(static_cast<ValueType&&>(value)))
        {
        }
#endif
        
        ~any() noexcept
        {
            delete content;
        }
        
    public: // modifiers
        
        any & swap(any & rhs) noexcept
        {
            std::swap(content, rhs.content);
            return *this;
        }
        
        
#ifdef NGN_NO_CXX11_RVALUE_REFERENCES
        template<typename ValueType>
        any & operator=(const ValueType & rhs)
        {
            any(rhs).swap(*this);
            return *this;
        }
        
        any & operator=(any rhs)
        {
            any(rhs).swap(*this);
            return *this;
        }
        
#else
        any & operator=(const any& rhs)
        {
            any(rhs).swap(*this);
            return *this;
        }
        
        // move assignement
        any & operator=(any&& rhs) noexcept
        {
            rhs.swap(*this);
            any().swap(rhs);
            return *this;
        }
        
        // Perfect forwarding of ValueType
        template <class ValueType>
        any & operator=(ValueType&& rhs)
        {
            any(static_cast<ValueType&&>(rhs)).swap(*this);
            return *this;
        }
#endif
        
    public: // queries
        
        bool empty() const noexcept
        {
            return !content;
        }
        
        const std::type_info & type() const
        {
            return content ? content->type() : typeid(void);
        }
        
#ifndef NGN_NO_MEMBER_TEMPLATE_FRIENDS
    private: // types
#else
    public: // types (public so any_cast can be non-friend)
#endif
        
        class placeholder
        {
        public: // structors
            
            virtual ~placeholder()
            {
            }
            
        public: // queries
            
            virtual const std::type_info & type() const = 0;
            
            virtual placeholder * clone() const = 0;
            
        };
        
        template<typename ValueType>
        class holder : public placeholder
        {
        public: // structors
            
            holder(const ValueType & value)
            : held(value)
            {
            }
            
#ifndef NGN_NO_CXX11_RVALUE_REFERENCES
            holder(ValueType&& value)
            : held(static_cast< ValueType&& >(value))
            {
            }
#endif
        public: // queries
            
            virtual const std::type_info & type() const
            {
                return typeid(ValueType);
            }
            
            virtual placeholder * clone() const
            {
                return new holder(held);
            }
            
        public: // representation
            
            ValueType held;
            
        private: // intentionally left unimplemented
            holder & operator=(const holder &);
        };
        
#ifndef NGN_NO_MEMBER_TEMPLATE_FRIENDS
        
    private: // representation
        
        template<typename ValueType>
        friend ValueType * any_cast(any *) noexcept;
        
        template<typename ValueType>
        friend ValueType * unsafe_any_cast(any *) noexcept;
        
#else
        
    public: // representation (public so any_cast can be non-friend)
        
#endif
        
        placeholder * content;
        
    };
    
    inline void swap(any & lhs, any & rhs) noexcept
    {
    lhs.swap(rhs);
}

class bad_any_cast : public std::bad_cast
{
public:
    virtual const char * what() const throw()
    {
        return "boost::bad_any_cast: "
        "failed conversion using boost::any_cast";
    }
};

template<typename ValueType>
ValueType * any_cast(any * operand) noexcept
{
return operand &&
#ifdef NGN_AUX_ANY_TYPE_ID_NAME
std::strcmp(operand->type().name(), typeid(ValueType).name()) == 0
#else
operand->type() == typeid(ValueType)
#endif
? &static_cast<any::holder<ValueType> *>(operand->content)->held
: 0;
}

template<typename ValueType>
inline const ValueType * any_cast(const any * operand) noexcept
{
return any_cast<ValueType>(const_cast<any *>(operand));
}

template<typename ValueType>
ValueType any_cast(any & operand)
{
    typedef typename remove_reference<ValueType>::type nonref;
    
    // If 'nonref' is still reference type, it means the user has not
    // specialized 'remove_reference'.
    
    // Please use NGN_BROKEN_COMPILER_TYPE_TRAITS_SPECIALIZATION macro
    // to generate specialization of remove_reference for your class
    // See type traits library documentation for details
   /* static_assert(is_reference<nonref>::value,
                  "remove_reference<ValueType> returned a reference type. You must define a specialization for this type");*/
    
    nonref * result = any_cast<nonref>(&operand);
    static_assert(!is_reference<nonref>::value,
                  "remove_reference<ValueType> returned a reference type. You must define a specialization for this type");
    if(!result)
        throw bad_any_cast();
    return *result;
}

template<typename ValueType>
inline ValueType any_cast(const any & operand)
{
    typedef typename remove_reference<ValueType>::type nonref;
    

    static_assert(!is_reference<nonref>::value,
                  "remove_reference<ValueType> returned a reference type. You must define a specialization for this type");
    
    return any_cast<const nonref &>(const_cast<any &>(operand));
}

// Note: The "unsafe" versions of any_cast are not part of the
// public interface and may be removed at any time. They are
// required where we know what type is stored in the any and can't
// use typeid() comparison, e.g., when our types may travel across
// different shared libraries.
template<typename ValueType>
inline ValueType * unsafe_any_cast(any * operand) noexcept
{
return &static_cast<any::holder<ValueType> *>(operand->content)->held;
}

template<typename ValueType>
inline const ValueType * unsafe_any_cast(const any * operand) noexcept
{
return unsafe_any_cast<ValueType>(const_cast<any *>(operand));
}
}

// Copyright Kevlin Henney, 2000, 2001, 2002. All rights reserved.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#endif
