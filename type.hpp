#ifndef __TYPE_HPP__
#define __TYPE_HPP__

#if __cplusplus < 201103L

typedef char               SInt8;
typedef short              SInt16;
typedef int                SInt32;
typedef long long          SInt64;
typedef unsigned char      UInt8;
typedef unsigned short     UInt16;
typedef unsigned int       UInt32;
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

#include <climits>

static int const BIT_PER_CHAR = CHAR_BIT;

template<typename type>
struct BitwidthOf {
	typedef type Type;
	enum { value = sizeof(Type) * BIT_PER_CHAR };
};

#endif // __TYPE_HPP__
