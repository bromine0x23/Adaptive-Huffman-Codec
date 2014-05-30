#ifndef __CODEC_HPP__
#define __CODEC_HPP__

#include "type.hpp"
#include "bit_math.hpp"

#include <cassert>
#include <memory>

template<typename symbol_type>
class CodecBase {
private:
	using Self = CodecBase;

public:
	using Symbol = symbol_type;

	static Size const SYMBOL_BIT = BitSize<Symbol>::value;
	static Size const SYMBOL_NUM = static_cast<Size>(1) << SYMBOL_BIT;
};

template<typename symbol_type>
class Encoder : public CodecBase<symbol_type> {
private:
	using Self = Encoder;

public:
	using Symbol = symbol_type;

	using Base = CodecBase<Symbol>;

	using IStream = typename IO<Symbol>::IStream;
	using OStream = typename IO<Byte>::OStream;

protected:
	static Size const BUFFER_SIZE = 256;
	static Size const BUFFER_BIT = BUFFER_SIZE * BitSize<Byte>::value;

	OStream & ostream;
	Byte buffer[BUFFER_SIZE];
	Size buffer_bit;
	Size symbol_count;

public:
	Encoder(OStream & os) : ostream(os), symbol_count(0) {
		clear_buffer();
	}

	virtual ~Encoder() {
		flush();
		put_count();
	}

	virtual Self & put(Symbol symbol) {
		put_plain(symbol);
		++symbol_count;
		return *this;
	};

	Size count() {
		return symbol_count;
	}

	Self & operator<<(Symbol symbol) {
		return put(symbol);
	}

protected:
	void clear_buffer() {
		std::uninitialized_fill(buffer, buffer + BUFFER_SIZE, static_cast<Byte>(0));
		buffer_bit = 0;
	}

	void flush() {
		ostream.write(buffer, (buffer_bit + BitSize<Byte>::value - 1) / BitSize<Byte>::value);
		buffer_bit = 0;
	}

	void put_count() {
		ostream.write(reinterpret_cast<Byte *>(&symbol_count), sizeof(symbol_count));
	}

	void put_plain(Symbol symbol) {
		for (int i = 0; i < Base::SYMBOL_BIT; ++i) {
			put_bit(get_bit(symbol, i));
		}
	}

	void put_bit(Bit bit) {
		set_bit(buffer, buffer_bit++, bit);
		if (buffer_bit == BUFFER_BIT) {
			ostream.write(buffer, BUFFER_SIZE);
			clear_buffer();
		}
	}

private:
	Encoder(Self const &) = delete;
	Self & operator=(Self const &) = delete;
};

template<typename symbol_type>
class Decoder : public CodecBase<symbol_type> {
private:
	using Self = Decoder;

public:
	using Symbol = symbol_type;

	using Base = CodecBase<Symbol>;

	using IStream = typename IO<Byte>::IStream;
	using OStream = typename IO<Symbol>::OStream;

protected:
	static Size const BUFFER_SIZE = 256;
	static Size const BUFFER_BITWIDTH = BUFFER_SIZE * BIT_PER_BYTE;

	IStream & istream;
	Byte buffer[BUFFER_SIZE];
	Size buffer_bit;
	Size buffer_used;
	Size symbol_count;

public:
	Decoder(IStream & is) : istream(is), buffer_bit(0), buffer_used(0), symbol_count(0) {
		get_count();
	}

	virtual ~Decoder() {
		// do nothing
	}

	virtual Symbol get() {
		Symbol symbol = get_plain();
		--symbol_count;
		return symbol;
	};

	Self & operator>>(Symbol & symbol) {
		symbol = get();
		return *this;
	}

	virtual bool is_good() {
		return symbol_count > 0;
	}

	explicit operator bool() const {
		return is_good();
	}

protected:
	void fill_buffer() {
		istream.read(buffer, BUFFER_SIZE);
		buffer_bit = istream.gcount() * BIT_PER_CHAR;
		buffer_used = 0;
	}

	void get_count() {
		auto pos = istream.tellg();
		istream.seekg(-ByteSize<Size>::value, std::ios_base::end);
		istream.read(reinterpret_cast<Byte *>(&symbol_count), ByteSize<Size>::value);
		istream.seekg(pos);
	}

	Symbol get_plain() {
		Symbol symbol = 0;
		for (int i = 0; i < Base::SYMBOL_BIT; ++i) {
			set_bit(symbol, i, get_bit());
		}
		return symbol;
	}

	Bit get_bit() {
		if (buffer_used >= buffer_bit) {
			fill_buffer();
		}
		return ::get_bit(buffer, buffer_used++);
	}

private:
	Decoder(Self const &) = delete;
	Self & operator=(Self const &) = delete;
};

template<typename symbol_type = Byte, typename encoder_type = Encoder<symbol_type>, typename decoder_type = Decoder<symbol_type>>
class Codec : public CodecBase<symbol_type> {
private:
	using Self = Codec;

public:
	using Symbol = symbol_type;

	using Base = CodecBase<Symbol>;

	using Encoder = encoder_type;
	using Decoder = decoder_type;

	using ISymbolFileStream = typename IO<Symbol>::IFileStream;
	using OSymbolFileStream = typename IO<Symbol>::OFileStream;

	using IByteFileStream = typename IO<Byte>::IFileStream;
	using OByteFileStream = typename IO<Byte>::OFileStream;

	static void encode(typename Encoder::IStream & istream, typename Encoder::OStream & ostream) {
		Encoder encoder(ostream);
		for (Symbol symbol; istream.get(symbol).good();) {
			encoder.put(symbol);
		}
	}

	static void encode(char const * input_file, char const * output_file) {
		ISymbolFileStream fin(input_file, std::ios::binary);
		OByteFileStream fout(output_file, std::ios::binary);
		encode(fin, fout);
	}

	static void decode(typename Decoder::IStream & istream, typename Decoder::OStream & ostream) {
		for (Decoder decoder(istream); decoder.is_good();) {
			ostream.put(decoder.get());
		}
	}

	static void decode(char const * input_file, char const * output_file) {
		IByteFileStream fin(input_file, std::ios::binary);
		OSymbolFileStream fout(output_file, std::ios::binary);
		decode(fin, fout);
	}

};

#endif
