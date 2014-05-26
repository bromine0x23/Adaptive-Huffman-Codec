#ifndef __BIT_MATH_HPP__
#define __BIT_MATH_HPP__

#include "type.hpp"

enum class Bit { zero = 0, one = 1 };

template<typename value_type>
inline value_type low_bit(value_type value) {
	return value & (~value + 1);
}

template<typename value_type>
inline Bit bit_at(value_type value, Size index) {
	return static_cast<Bit>((value >> index) & static_cast<value_type>(0x1));
}

template<typename value_type>
inline value_type push_bit(value_type value, Bit bit) {
	return value << static_cast<value_type>(1) | static_cast<value_type>(bit);
}

template<typename value_type>
inline void set_bit(value_type & value, Bit bit, Size index) {
	value |= static_cast<int>(bit) << index;
}

#endif // __BIT_MATH_HPP__