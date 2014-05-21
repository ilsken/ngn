//
// TransformationTraits
// see: http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3655.pdf
//

#ifndef ngn_transform_traits_h
#define ngn_transform_traits_h

namespace ngn { namespace detail {

// 20.9.7.1, const-volatile modifications:
template <class T>
using remove_const_t = typename std::remove_const<T>::type;
template <class T>
using remove_volatile_t = typename std::remove_volatile<T>::type;
template <class T>
using remove_cv_t = typename std::remove_cv<T>::type;
template <class T>
using add_const_t = typename std::add_const<T>::type;
template <class T>
using add_volatile_t = typename std::add_volatile<T>::type;
template <class T>
using add_cv_t = typename std::add_cv<T>::type;

// 20.9.7.2, reference modifications:
template <class T>
using remove_reference_t = typename std::remove_reference<T>::type;
template <class T>
using add_lvalue_reference_t = typename std::add_lvalue_reference<T>::type;
template <class T>
using add_rvalue_reference_t = typename std::add_rvalue_reference<T>::type;

// 20.9.7.3, sign modifications:
template <class T>
using make_signed_t = typename std::make_signed<T>::type;
template <class T>
using make_unsigned_t = typename std::make_unsigned<T>::type;

// 20.9.7.4, array modifications:
template <class T>
using remove_extent_t = typename std::remove_extent<T>::type;
template <class T>
using remove_all_extents_t = typename std::remove_all_extents<T>::type;

// 20.9.7.5, pointer modifications:
template <class T>
using remove_pointer_t = typename std::remove_pointer<T>::type;
template <class T>
using add_pointer_t = typename std::add_pointer<T>::type;

// 20.9.7.6, other transformations:
template <size_t Len, std::size_t Align = alignof(std::max_align_t)> // see 20.9.7.6
using aligned_storage_t = typename std::aligned_storage<Len,Align>::type;
template <std::size_t Len, class... Types>
using aligned_union_t = typename std::aligned_union<Len,Types...>::type;
template <class T>
using decay_t = typename std::decay<T>::type;
template <bool b, class T=void>
using enable_if_t = typename std::enable_if<b,T>::type;
template <bool b, class T, class F>
using conditional_t = typename std::conditional<b,T,F>::type;
template <class... T>
using common_type_t = typename std::common_type<T...>::type;
template <class T>
using underlying_type_t = typename std::underlying_type<T>::type;
template <class T>
using result_of_t = typename std::result_of<T>::type;

// 20.9.4.1, primary type categories:
template <class T>
using is_void_t = typename std::is_void<T>::type;
template <class T>
using is_integral_t = typename std::is_integral<T>::type;
template <class T>
using is_floating_point_t = typename std::is_floating_point<T>::type;
template <class T>
using is_array_t = typename std::is_array<T>::type;
template <class T>
using is_pointer_t = typename std::is_pointer<T>::type;
template <class T>
using is_lvalue_reference_t = typename std::is_lvalue_reference<T>::type;
template <class T>
using is_rvalue_reference_t = typename std::is_rvalue_reference<T>::type;
template <class T>
using is_member_object_pointer_t = typename std::is_member_object_pointer
    
    <T>::type;
template <class T>
using is_member_function_pointer_t = typename std::is_member_function_pointer<T>::type;
template <class T>
using is_enum_t = typename std::is_enum<T>::type;
template <class T>
using is_union_t = typename std::is_union<T>::type;
template <class T>
using is_class_t = typename std::is_class<T>::type;
template <class T>
using is_function_t = typename std::is_function<T>::type;
// 20.9.4.2, composite type categories:
template <class T>
using is_reference_t = typename std::is_reference<T>::type;
template <class T>
using is_arithmetic_t = typename std::is_arithmetic<T>::type;
template <class T>
using is_fundamental_t = typename std::is_fundamental<T>::type;
template <class T>
using is_object_t = typename std::is_object<T>::type;
template <class T>
using is_scalar_t = typename std::is_scalar<T>::type;
template <class T>
using is_compound_t = typename std::is_compound<T>::type;
template <class T>
using is_member_pointer_t = typename std::is_member_pointer<T>::type;
// 20.9.4.3, type properties:
template <class T>
using is_const_t = typename std::is_const<T>::type;
template <class T>
using is_volatile_t = typename std::is_volatile<T>::type;
template <class T>
using is_trivial_t = typename std::is_trivial<T>::type;
template <class T>
using is_trivially_copyable_t = typename std::is_trivially_copyable<T>::type;
template <class T>
using is_standard_layout_t = typename std::is_standard_layout<T>::type;
template <class T>
using is_pod_t = typename std::is_pod<T>::type;
template <class T>
using is_literal_type_t = typename std::is_literal_type<T>::type;
template <class T>
using is_empty_t = typename std::is_empty<T>::type;
template <class T>
using is_polymorphic_t = typename std::is_polymorphic<T>::type;
template <class T>
using is_abstract_t = typename std::is_abstract<T>::type;
template <class T>
using is_signed_t = typename std::is_signed<T>::type;
template <class T>
using is_unsigned_t = typename std::is_unsigned<T>::type;
template <class T, class... Args>
using is_constructible_t = typename std::is_constructible<T, Args...>::type;
template <class T>
using is_default_constructible_t = typename std::is_default_constructible<T>::type;
template <class T>
using is_copy_constructible_t = typename std::is_copy_constructible<T>::type;
template <class T>
using is_move_constructible_t = typename std::is_move_constructible<T>::type;
template <class T, class U>
using is_assignable_t = typename std::is_assignable<T, U>::type;
template <class T>
using is_copy_assignable_t = typename std::is_copy_assignable<T>::type;
template <class T>
using is_move_assignable_t = typename std::is_move_assignable<T>::type;
template <class T>
using is_destructible_t = typename std::is_destructible<T>::type;
template <class T, class... Args>
using is_trivially_constructible_t = typename std::is_trivially_constructible<T, Args...>::type;
template <class T>
using is_trivially_default_constructible_t = typename std::is_trivially_default_constructible<T>::type;
template <class T>
using is_trivially_copy_constructible_t
= typename std::is_trivially_copy_constructible<T>::type;
template <class T>
using is_trivially_move_constructible_t
= typename std::is_trivially_move_constructible<T>::type;
template <class T, class U>
using is_trivially_assignable_t
= typename std::is_trivially_assignable<T, U>::type;
template <class T>
using is_trivially_copy_assignable_t
= typename std::is_trivially_copy_assignable<T>::type;
template <class T>
using is_trivially_move_assignable_t
= typename std::is_trivially_move_assignable<T>::type;
template <class T>
using is_trivially_destructible_t
= typename std::is_trivially_destructible<T>::type;
template <class T, class... Args>
using is_nothrow_constructible_t
= typename std::is_nothrow_constructible<T, Args...>::type;
template <class T>
using is_nothrow_default_constructible_t
= typename std::is_nothrow_default_constructible<T>::type;
template <class T>
using is_nothrow_copy_constructible_t
= typename std::is_nothrow_copy_constructible<T>::type;
template <class T>
using is_nothrow_move_constructible_t
= typename std::is_nothrow_move_constructible<T>::type;
template <class T, class U>
using is_nothrow_assignable_t = typename std::is_nothrow_assignable<T, U>::type;
template <class T>
using is_nothrow_copy_assignable_t = typename std::is_nothrow_copy_assignable<T>::type;
template <class T>
using is_nothrow_move_assignable_t = typename std::is_nothrow_move_assignable<T>::type;
template <class T>
using is_nothrow_destructible_t = typename std::is_nothrow_destructible<T>::type;
template <class T>
using has_virtual_destructor_t = typename std::has_virtual_destructor<T>::type;
// 20.9.5, type property queries:
template <class T>
using alignment_of_t = typename std::alignment_of<T>::type;
template <class T, unsigned I = 0>
using extent_t = typename std::extent<T, I>::type;
// 20.9.6, type relations:
template <class T, class U>
using is_same_t = typename std::is_same<T, U>::type;
template <class Base, class Derived>
using is_base_of_t = typename std::is_base_of<Base, Derived>::type;
template <class From, class To>
using is_convertible_t = typename std::is_convertible<From, To>::type;
}}


#endif
