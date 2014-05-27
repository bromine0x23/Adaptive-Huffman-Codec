#ifndef __TYPE_HPP__
#define __TYPE_HPP__

#if __cplusplus < 201103L

typedef signed char        SInt8;
typedef signed short       SInt16;
typedef signed long        SInt32;
typedef signed long long   SInt64;
typedef unsigned char      UInt8;
typedef unsigned short     UInt16;
typedef unsigned long      UInt32;
typedef unsigned long long UInt64;

#else

#include <cstdint>

typedef int8_t  SInt8;
typedef int16_t SInt16;
typedef int32_t SInt32;
typedef int64_t SInt64;

typedef uint8_t  UInt8;
typedef uint16_t UInt16;
typedef uint32_t UInt32;
typedef uint64_t UInt64;

#endif

#include <cstddef>
#include <climits>

typedef unsigned char Byte;
typedef char Char;
typedef UInt64 Size;

static int const BIT_PER_BYTE = CHAR_BIT;
static int const BIT_PER_CHAR = CHAR_BIT;

template<typename type, type val>
struct IntegralConstant {
	enum { value = val };
	typedef type Value;
	typedef IntegralConstant<Value, value> Type;
	operator Value() { return value; }
};

typedef IntegralConstant<bool, true> True;
typedef IntegralConstant<bool, false> False;

template<typename>
struct IsSigned : False {};

template<> struct IsSigned<signed char>      : True {};
template<> struct IsSigned<signed short>     : True {};
template<> struct IsSigned<signed int>       : True {};
template<> struct IsSigned<signed long>      : True {};
template<> struct IsSigned<signed long long> : True {};
template<> struct IsSigned<unsigned char>      : False {};
template<> struct IsSigned<unsigned short>     : False {};
template<> struct IsSigned<unsigned int>       : False {};
template<> struct IsSigned<unsigned long>      : False {};
template<> struct IsSigned<unsigned long long> : False {};

template<typename>
struct IsUnsigned : False {};

template<> struct IsUnsigned<signed char>      : False {};
template<> struct IsUnsigned<signed short>     : False {};
template<> struct IsUnsigned<signed int>       : False {};
template<> struct IsUnsigned<signed long>      : False {};
template<> struct IsUnsigned<signed long long> : False {};
template<> struct IsUnsigned<unsigned char>      : True {};
template<> struct IsUnsigned<unsigned short>     : True {};
template<> struct IsUnsigned<unsigned int>       : True {};
template<> struct IsUnsigned<unsigned long>      : True {};
template<> struct IsUnsigned<unsigned long long> : True {};

template<typename type>
struct Signed {
	using Type = type;
};

template<> struct Signed<char>               { using Type = signed char;      };
template<> struct Signed<unsigned char>      { using Type = signed char;      };
template<> struct Signed<unsigned short>     { using Type = signed short;     };
template<> struct Signed<unsigned int>       { using Type = signed int;       };
template<> struct Signed<unsigned long>      { using Type = signed long;      };
template<> struct Signed<unsigned long long> { using Type = signed long long; };

template<typename type>
struct Unsigned {
	using Type = type;
};

template<> struct Unsigned<char>             { using Type = unsigned char;      };
template<> struct Unsigned<signed char>      { using Type = unsigned char;      };
template<> struct Unsigned<signed short>     { using Type = unsigned short;     };
template<> struct Unsigned<signed int>       { using Type = unsigned int;       };
template<> struct Unsigned<signed long>      { using Type = unsigned long;      };
template<> struct Unsigned<signed long long> { using Type = unsigned long long; };

template<typename value_type>
typename Signed<value_type>::Type neg(value_type value) {
	return - static_cast<typename Signed<value_type>::Type>(value);
}

template<typename>
struct IsArray : public False {};

template<typename type, Size size>
struct IsArray<type[size]> : public True {};

template<typename type>
struct IsArray<type[]> : public True {};

#include <string>
#include <ios>
#include <streambuf>
#include <istream>
#include <ostream>
#include <fstream>
#include <sstream>

template<typename type>
struct IO {
public:
	typedef type Char;

	typedef std::char_traits<Char> CharTraits;

	typedef std::basic_ios<Char> Stream;

	typedef std::basic_istream<Char> IStream;
	typedef std::basic_ostream<Char> OStream;
	typedef std::basic_iostream<Char> IOStream;

	typedef std::basic_ifstream<Char> IFileStream;
	typedef std::basic_ofstream<Char> OFileStream;
	typedef std::basic_fstream<Char> FileStream;

	typedef std::basic_istringstream<Char> IStringStream;
	typedef std::basic_ostringstream<Char> OStringStream;
	typedef std::basic_ostringstream<Char> StringStream;

	typedef std::basic_streambuf<Char> StreamBuffer;
	typedef std::basic_filebuf<Char> FileBuffer;
	typedef std::basic_stringbuf<Char> StringBuffer;
};

#include <deque>
#include <list>
#include <vector>
#include <set>

template<typename type>
struct Container {
	typedef type Value;

	typedef std::deque<Value> Deque;
	typedef std::list<Value> List;
	typedef std::vector<Value> Vector;
	typedef std::set<Value> Set;
	typedef std::multiset<Value> MultiSet;
};

template<typename type>
struct BitwidthOf {
	typedef type Type;
	enum { value = sizeof(Type) * CHAR_BIT };
};

#endif // __TYPE_HPP__
