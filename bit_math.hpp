#ifndef __BIT_MATH_HPP__
#define __BIT_MATH_HPP__

#include "type.hpp"

enum class Bit { zero = 0, one = 1 };

template<typename value_type>
inline value_type low_bit(value_type const & value) {
	return value & (~value + 1);
}

template<typename value_type, typename index_type>
inline Bit bit_at(value_type value, index_type index) {
	return static_cast<Bit>((value >> index) & 0x1);
}

template<typename value_type>
inline value_type operator<<(value_type const & value, Bit bit) {
	return (value << 1) | static_cast<value_type>(bit);
}

template<typename value_type>
inline value_type & operator<<=(value_type & value, Bit bit) {
	return value = (value << 1) | static_cast<value_type>(bit);
}

template<typename value_type, typename index_type>
inline value_type & set_bit(value_type & value, index_type index, Bit bit) {
	return value |= static_cast<int>(bit) << index;
}

#endif // __BIT_MATH_HPP__