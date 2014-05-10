#ifndef __ADAPTIVE_HUFFMAN_HPP__
#define __ADAPTIVE_HUFFMAN_HPP__

#include "type.hpp"

#include <cassert>
#include <cstring>
#include <cstddef>
#include <utility>
#include <memory>
#include <ostream>

#include <cctype>

class AdaptiveHuffmanTreeBase {
public:
	typedef SInt64 Symbol;
	typedef UInt64 Counter;

protected:
	struct Node;

	typedef Node * NodePointer;
	typedef Node const * ConstantNodePointer;
	typedef NodePointer Linker;
	typedef Linker * LinkerPointer;

	struct Node {
		Symbol symbol;
		Counter weight;
		NodePointer parent, left, right; /* tree structure */
		LinkerPointer block_head; /* highest ranked node in block */
		Linker next, prev; /* doubly-linked list */
	};


private:
	LinkerPointer free_linkers;

public:
	AdaptiveHuffmanTreeBase();

	virtual ~AdaptiveHuffmanTreeBase();

protected:
	LinkerPointer get_linker(Linker init_value);

	void free_linker(LinkerPointer linker);

public:
	class Cursor {
	private:
		typedef Cursor Self;

		ConstantNodePointer node;

	public:
		Cursor(ConstantNodePointer node) : node(node) {}

		Symbol symbol() const {
			return node->symbol;
		}

		bool is_null() const {
			return node == NULL;
		}

		bool is_root() const {
			return node->parent == NULL;
		}

		bool is_left_child() const {
			return node->parent->left == node;
		}

		bool is_right_child() const {
			return node->parent->right == node;
		}

		Self parent() const {
			return Self(node->parent);
		}

		Self left() const {
			return Self(node->left);
		}

		Self right() const {
			return Self(node->right);
		}

		Self & up() {
			node = node->parent;
			return *this;
		}

		Self & down_left() {
			node = node->left;
			return *this;
		}

		Self & down_right() {
			node = node->right;
			return *this;
		}
	};
};

template<typename type = char>
class AdaptiveHuffmanTree : public AdaptiveHuffmanTreeBase {
private:
	typedef AdaptiveHuffmanTreeBase Base;
	typedef AdaptiveHuffmanTree Self;

public:
	typedef type Char;

public:
	static int const SYMBOL_BIT = BitwidthOf<Char>::value;
	static int const SYMBOL_NUM = 1 << SYMBOL_BIT;
	static int const NYT_SYMBOL = SYMBOL_NUM; // symbol of not yet transmitted
	static int const INTERNAL_SYMBOL = SYMBOL_NUM + 1; // symbol of internal node

private:
	NodePointer tree_root;
	Linker list_head;
	Linker location[SYMBOL_NUM + 1];

public:
	AdaptiveHuffmanTree();

	~AdaptiveHuffmanTree();

	Cursor root() const {
		return Cursor(tree_root);
	}

	Cursor node(Symbol symbol) const {
		return Cursor(location[symbol]);
	}

	void update(Symbol symbol);

private:
	void add_symbol(Symbol symbol);

	// Do the increments
	void increment(NodePointer node);


	NodePointer get_node(Symbol symbol);

	void push_head(NodePointer node);

	// Swap the location of these two nodes in the tree
	void swap_in_tree(NodePointer node1, NodePointer node2);

	// Swap these two nodes in the linked list (update ranks)
	void swap_in_list(NodePointer node1, NodePointer node2);

public:
	void check_rank() const;

	// Debugging routine...dump the tree in prefix notation
	void dump_tree(std::basic_ostream<Char> & os) const;

	// Debugging routine...dump the link list
	void dump_list(std::basic_ostream<Char> & os) const;

private:
	void dump_tree(std::basic_ostream<Char> & os, ConstantNodePointer node) const;

private:
	AdaptiveHuffmanTree(Self const &);
	Self & operator=(Self const &);
};

template<typename type = char>
class AdaptiveHuffmanCodecBase {
private:
	typedef AdaptiveHuffmanCodecBase Self;

public:
	typedef char Bit;
	typedef type Char;
	typedef AdaptiveHuffmanTree<Char> Tree;

	static int const BUFFER_SIZE = 256;
	static int const BIT_PER_CHAR = BitwidthOf<Char>::value;
	static int const CHAR_NUM = 1 << BIT_PER_CHAR;
};

template<typename type = char>
class AdaptiveHuffmanEncoder : public AdaptiveHuffmanCodecBase<type> {
private:
	typedef AdaptiveHuffmanEncoder Self;

public:
	typedef AdaptiveHuffmanCodecBase<type> Base;
	typedef typename Base::Char Char;
	typedef typename Base::Bit Bit;
	typedef typename Base::Tree Tree;
	typedef typename Tree::Symbol Symbol;
	typedef typename Tree::Counter Counter;
	typedef typename Tree::Cursor Cursor;
	typedef typename IO<Char>::OStream OStream;

	using Base::BUFFER_SIZE;
	using Base::BIT_PER_CHAR;
	using Base::CHAR_NUM;

private:
	OStream & ostream;
	Tree tree;
	Counter symbol_count;
	Counter bit_count;
	Char write_buffer[Base::BUFFER_SIZE];

public:
	AdaptiveHuffmanEncoder(OStream & os);

	Self & put(Symbol symbol);

	Self & put_tail();

	void flush();

	Self & operator<<(Symbol symbol);

	Counter count() const;

private:
	void clear_buffer();

	// Send a symbol
	void put_symbol(Symbol symbol);

	// Send the prefix code for this node
	void encode(Cursor cursor);

	// Add a bit to the output file (buffered)
	void put_bit(Bit bit);

private:
	AdaptiveHuffmanEncoder(Self const &);
	Self & operator=(Self const &);
};

template<typename type = char>
class AdaptiveHuffmanDecoder : public AdaptiveHuffmanCodecBase<type> {
private:
	typedef AdaptiveHuffmanDecoder Self;

public:
	typedef AdaptiveHuffmanCodecBase<type> Base;
	typedef typename Base::Char Char;
	typedef typename Base::Bit Bit;
	typedef typename Base::Tree Tree;
	typedef typename Tree::Symbol Symbol;
	typedef typename Tree::Counter Counter;
	typedef typename Tree::Cursor Cursor;
	typedef typename IO<Char>::IStream IStream;

	using Base::BUFFER_SIZE;
	using Base::BIT_PER_CHAR;
	using Base::CHAR_NUM;

private:
	IStream & istream;
	Tree tree;
	Counter symbol_count;
	Counter bit_count;
	Char read_buffer[Base::BUFFER_SIZE];

public:
	AdaptiveHuffmanDecoder(IStream & is);

	Counter count() const;

	Symbol get();

private:
	// Get a symbol
	Symbol get_symbol();

	// Receive one bit from the input file (buffered)
	Bit get_bit();

private:
	AdaptiveHuffmanDecoder(Self const &);
	Self & operator=(Self const &);
};

template<typename type = char>
class AdaptiveHuffmanCodec : public AdaptiveHuffmanCodecBase<type> {
private:
	typedef AdaptiveHuffmanCodec Self;

public:
	typedef AdaptiveHuffmanCodecBase<type> Base;
	typedef typename Base::Char Char;
	typedef typename Base::Tree Tree;
	typedef typename Tree::Symbol Symbol;
	typedef AdaptiveHuffmanEncoder<Char> Encoder;
	typedef AdaptiveHuffmanDecoder<Char> Decoder;
	typedef typename Encoder::OStream OStream;
	typedef typename Decoder::IStream IStream;
	typedef typename IO<Char>::OFileStream OFileStream;
	typedef typename IO<Char>::IFileStream IFileStream;

	using Base::BUFFER_SIZE;
	using Base::BIT_PER_CHAR;
	using Base::CHAR_NUM;

	static int const CHAR_NUM_HALF = CHAR_NUM / 2;

public:
	static void encode_to(IStream & istream, OStream & ostream);
	static void decode_from(OStream & ostream, IStream & istream);
	static void encode_to(char const * input_file, char const * output_file);
	static void decode_from(char const * input_file, char const * output_file);
};

#endif // __ADAPTIVE_HUFFMAN_HPP__
