/*
This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef __TYPES_H_
#define __TYPES_H_

#include <stddef.h>
#include <stdint.h>

/*
  define some specific length types
*/
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

// Helper functions for templating

template <typename, typename>
constexpr bool is_same_v = false;
template <typename T>
constexpr bool is_same_v<T, T> = true;

template<typename T, typename... Ts>
constexpr bool is_any_of_v = (is_same_v<T, Ts> || ...);

template <typename T>
struct remove_const_volatile {
	using type = T;
};

template <typename T>
struct remove_const_volatile<volatile T> {
	using type = T;
};

template <typename T>
struct remove_const_volatile<const T> {
	using type = T;
};

template <typename T>
struct remove_const_volatile<const volatile T> {
	using type = T;
};

template<typename T>
using remove_cv_t = typename remove_const_volatile<T>::type;

template<typename Ty>
constexpr bool is_int_like_v = is_any_of_v<
	remove_cv_t<Ty>,
	char,
	signed char,
	unsigned char,
	wchar_t,
	char16_t,
	char32_t,
	short,
	unsigned short,
	int,
	unsigned int,
	long,
	unsigned long,
	long long,
	unsigned long long
>;

template <class T>
concept int_like = is_int_like_v<T>; //Concept isn't reverse compatible so requires C++ 20 which isn't fully supported on some compilers just fyi

#endif
