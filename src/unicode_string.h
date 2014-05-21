//
//  string.h
//  ngn
//
//  Created by Christopher Tarquini on 2/4/14.
//
//

#ifndef __ngn__string__
#define __ngn__string__

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <algorithm>
#include <string>

namespace ngn {
    using std::size_t;
    typedef char utf8_code_unit;
    typedef char ascii_code_unit;
    typedef char16_t utf16_code_unit;
    typedef char32_t utf32_code_unit;
    typedef char32_t unicode_char;
    
    
    template <class CodeUnit, size_t MaxSize>
    struct code_point {
        typedef CodeUnit code_unit;
        static const size_t max_size = MaxSize;
        code_unit code_units[max_size];
        bool operator==(const code_point<code_unit, max_size>& other) {
            return std::equal(code_units, other.code_units);
        }
    };
    typedef code_point<ascii_code_unit, 1> ascii_code_point;
    typedef code_point<utf8_code_unit, 4> utf8_code_point;
    typedef code_point<utf16_code_unit, 2 > utf16_code_point;
    typedef code_point<utf32_code_unit, 1> utf32_code_point;
    
    template <class CodePoint>
    struct  code_unit_traits : std::char_traits<typename CodePoint::code_unit> {
        typedef CodePoint code_point;
        typedef typename code_point::code_unit code_unit;
        typedef unicode_char character_type;
    };
    
    
}

#endif /* defined(__ngn__string__) */
