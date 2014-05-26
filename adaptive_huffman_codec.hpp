#ifndef __ADAPTIVE_HUFFMAN_CODEC_HPP__
#define __ADAPTIVE_HUFFMAN_CODEC_HPP__

#include "type.hpp"
#include "bit_math.hpp"

#include <cassert>
#include <cstring>
#include <cstddef>
#include <utility>
#include <memory>
#include <ostream>

#include <cctype>

template<typename symbol_type = char>
class AdaptiveHuffmanTree {
private:
	using Self = AdaptiveHuffmanTree;

public:
	using Symbol = symbol_type;
	using InternalSymbol = UInt64;

private:
	struct Node;

	using Linker = Node *;

	struct Node {
		InternalSymbol symbol;
		Size weight;
		Linker parent, left, right;
		Linker next, prev; /* doubly-linked list */
		Linker * block_head; /* highest ranked node in block */
	};

public:
	class Cursor {
	private:
		using Self = Cursor;

	private:
		Node const * node;

	public:
		Cursor(Node const * node) : node(node) {
			// do nothing
		}

		InternalSymbol symbol() const {
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

		Bit side() const {
			if (is_left_child()) {
				return Bit::zero;
			} else {
				assert(is_right_child());
				return Bit::one;
			}
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

		Self & down(Bit bit) {
			return bit == Bit::zero ? down_left() : down_right();
		}
	};

public:
	static Size const SYMBOL_BIT = BitwidthOf<Symbol>::value;
	static Size const SYMBOL_NUM = Size(1) << SYMBOL_BIT;
	static InternalSymbol const NYT_SYMBOL = SYMBOL_NUM; // id of not yet transmitted
	static InternalSymbol const INTERNAL_SYMBOL = SYMBOL_NUM + 1; // id of internal node

private:
	Linker * free_linkers;
	Node * tree_root;
	Linker list_head;
	Linker location[SYMBOL_NUM + 1];

public:
	AdaptiveHuffmanTree();

	virtual ~AdaptiveHuffmanTree();

	Cursor root() const {
		return Cursor(tree_root);
	}

	Cursor node(InternalSymbol symbol) const {
		return Cursor(location[symbol]);
	}

	Cursor nyt() const {
		return Cursor(location[NYT_SYMBOL]);
	}

	Self & operator<<(Symbol symbol);

private:
	void new_symbol(Symbol symbol);

	// Do the increments
	void increse_weight(Node * node);

	Node * get_node(InternalSymbol symbol) {
		Node * node = new Node;
		node->symbol = symbol;
		node->weight = 0;
		node->parent = node->left = node->right = NULL;
		node->block_head = NULL;
		node->next = node->prev = NULL;
		return node;
	}

	Linker * get_linker(Linker value) {
		Linker * linker;
		if (free_linkers != NULL) {
			linker = free_linkers;
			free_linkers = reinterpret_cast<Linker *>(*linker);
			*linker = value;
		} else {
			linker = new Linker(value);
		}
		return linker;
	}

	void put_linker(Linker * linker) {
		*linker = reinterpret_cast<Linker>(free_linkers);
		free_linkers = linker;
	}

	void push_head(Node * node) {
		node->next = list_head;
		list_head->prev = node;
		node->block_head = list_head->block_head;
		list_head = node;
	}

	// Swap the location of these two nodes in the tree
	void swap_in_tree(Node * node1, Node * node2);

	// Swap these two nodes in the linked list (update ranks)
	void swap_in_list(Node * node1, Node * node2);

#ifdef NDEBUG
public:
	void check_rank() const;
	// Debugging routine...dump the link list
	void dump_list() const;

	// Debugging routine...dump the tree in prefix notation
	void dump_tree() const;

private:
	void dump_tree(ConstantNodePointer node) const;
#endif // NDEBUG


private:
	AdaptiveHuffmanTree(Self const &) = delete;
	Self & operator=(Self const &) = delete;
}; // class AdaptiveHuffmanTree

template<typename type>
AdaptiveHuffmanTree<type>::AdaptiveHuffmanTree() : free_linkers(NULL) {
	for (int i = 0; i <= SYMBOL_NUM; ++i) {
		location[i] = NULL;
	}
	tree_root = list_head = location[NYT_SYMBOL] = get_node(NYT_SYMBOL);
	list_head->block_head = get_linker(list_head);
}

template<typename type>
AdaptiveHuffmanTree<type>::~AdaptiveHuffmanTree() {
	for (Node * node = list_head;;) {
		Node * old_node = node;
		node = node->next;
		if (node != NULL) {
			if (old_node->weight != node->weight) {
				delete old_node->block_head;
			}
			delete old_node;
		} else {
			break;
		}
	}
	for (Linker * linker = free_linkers; linker != NULL;) {
		Linker * old_linker = linker;
		linker = reinterpret_cast<Linker *>(*linker);
		delete old_linker;
	}
}

template<typename type>
auto AdaptiveHuffmanTree<type>::operator<<(Symbol symbol) -> Self & {
	InternalSymbol internal_symbol = static_cast<InternalSymbol>(symbol);
	if (location[internal_symbol] == NULL) {
		new_symbol(symbol);
	}
	increse_weight(location[internal_symbol]);
	return *this;
}

template<typename type>
void AdaptiveHuffmanTree<type>::new_symbol(Symbol symbol) {
	assert(list_head->symbol == NYT_SYMBOL);

	InternalSymbol internal_symbol = static_cast<InternalSymbol>(symbol);

	Node * symbol_node = get_node(internal_symbol);
	push_head(symbol_node);

	Node * new_nyt_node = get_node(NYT_SYMBOL);
	push_head(new_nyt_node);

	Node * old_nyt_node = location[NYT_SYMBOL];
	assert(old_nyt_node != NULL && old_nyt_node->left == NULL && old_nyt_node->right == NULL);
	old_nyt_node->symbol = INTERNAL_SYMBOL;

	new_nyt_node->parent = symbol_node->parent = old_nyt_node;
	old_nyt_node->left = new_nyt_node;
	old_nyt_node->right = symbol_node;

	location[internal_symbol] = symbol_node;
	location[NYT_SYMBOL] = new_nyt_node;
}

template<typename type>
void AdaptiveHuffmanTree<type>::increse_weight(Node * node) {
	assert(node != NULL);

	if (node->next != NULL && node->next->weight == node->weight) {
		Linker head = *node->block_head;
		assert(head != node && head->parent != node && head->weight == node->weight);
		if (head != node->parent) {
			swap_in_tree(head, node);
		} else {
			assert(node->next == head);
		}
		swap_in_list(head, node);
	}

	if (node->prev != NULL && node->weight == node->prev->weight) {
		*node->block_head = node->prev;
	} else {
		put_linker(node->block_head);
		node->block_head = NULL;
	}

	++node->weight;

	if (node->next != NULL && node->weight == node->next->weight) {
		node->block_head = node->next->block_head;
	} else {
		node->block_head = get_linker(node);
	}

	if (node->parent != NULL) {
		increse_weight(node->parent);
		if (node->prev == node->parent) {
			swap_in_list(node, node->parent);
			if (*node->block_head == node) {
				*node->block_head = node->parent;
			}
		}
	}
}

template<typename type>
void AdaptiveHuffmanTree<type>::swap_in_tree(Node * node1, Node * node2) {
	assert(node1->symbol != NYT_SYMBOL && node2->symbol != NYT_SYMBOL);

	Node * node1_parent = node1->parent;
	Node * node2_parent = node2->parent;

	if (node1_parent != NULL) {
		if (node1_parent->left == node1) {
			node1_parent->left = node2;
		} else {
			assert(node1_parent->right == node1);
			node1_parent->right = node2;
		}
	} else {
		tree_root = node2;
	}

	if (node2_parent != NULL) {
		if (node2_parent->left == node2) {
			node2_parent->left = node1;
		} else {
			assert(node2_parent->right == node2);
			node2_parent->right = node1;
		}
	} else {
		tree_root = node1;
	}

	node1->parent = node2_parent;
	node2->parent = node1_parent;
}

template<typename type>
void AdaptiveHuffmanTree<type>::swap_in_list(Node * node1, Node * node2) {
	std::swap(node1->next, node2->next);
	std::swap(node1->prev, node2->prev);

	if (node1->next == node1) {
		node1->next = node2;
	}
	if (node2->next == node2) {
		node2->next = node1;
	}

	if (node1->next != NULL) {
		node1->next->prev = node1;
	}
	if (node2->next != NULL) {
		node2->next->prev = node2;
	}

	if (node1->prev != NULL) {
		node1->prev->next = node1;
	}
	if (node2->prev != NULL) {
		node2->prev->next = node2;
	}

	assert(node1->next != node1);
	assert(node2->next != node2);
}

#ifdef NDEBUG
#include <iostream>

template<typename type>
void AdaptiveHuffmanTree<type>::check_rank() const {
	for (ConstantNodePointer node = list_head; node; node = node->next) {
		assert(node->next == NULL || node->weight <= node->next->weight);
		assert(node->block_head != NULL && *(node->block_head) != NULL && (*node->block_head)->weight == node->weight);
		if (node->next != NULL) {
			if (node->weight == node->next->weight) {
				assert(node->block_head == node->next->block_head);
			} else {
				assert(node->block_head != node->next->block_head);
			}
		}
	}
}

template<typename type>
void AdaptiveHuffmanTree<type>::dump_list() const {
	for (ConstantNodePointer node = list_head; node != NULL; node = node->next) {
		std::cout << '[' << node->symbol << ']' << '(' << node->weight << ')';
	}
	std::cout << std::endl;
}

template<typename type>
inline void AdaptiveHuffmanTree<type>::dump_tree() const {
	dump_tree(tree_root);
	std::cout << std::endl;
}

template<typename type>
void AdaptiveHuffmanTree<type>::dump_tree(ConstantNodePointer node) const {
	for (; node != NULL; node = node->right) {
		std::cout << '[' << node->symbol << ']';
		if (node->left != NULL) {
			assert(node->left->parent == node);
			dump_tree(node->left);
		}

		if (node->right != NULL) {
			assert(node->right->parent == node);
		}
	}
}
#endif // NDEBUG

#include "codec.hpp"

template<typename symbol_type = char>
class AdaptiveHuffmanEncoder : public Encoder<symbol_type> {
private:
	using Self = AdaptiveHuffmanEncoder;

public:
	using Symbol = symbol_type;

	using Base = Encoder<Symbol>;
	using ISymbolStream = typename Base::ISymbolStream;
	using OCharStream   = typename Base::OCharStream;

	using Tree = AdaptiveHuffmanTree<Symbol>;
	using InternalSymbol = typename Tree::InternalSymbol;
	using Cursor         =  typename Tree::Cursor;

	using Base::BUFFER_SIZE;
	using Base::BUFFER_BITWIDTH;

private:
	Tree tree;

public:
	AdaptiveHuffmanEncoder(OCharStream & os) : Base(os) {
		// do nothing
	}

	Self & put(Symbol symbol) {
		printf("put() -> %d\n", symbol);

		put_symbol(symbol);
		tree << symbol;
		++this->symbol_count;
		return *this;
	}

private:
	// Send a symbol
	void put_symbol(Symbol symbol);

	// Encode symbol and send code
	void encode_and_put(Cursor cursor);

private:
	AdaptiveHuffmanEncoder(Self const &);
	Self & operator=(Self const &);
}; // class AdaptiveHuffmanEncoder

template<typename type>
void AdaptiveHuffmanEncoder<type>::put_symbol(Symbol symbol) {
	printf("put_symbol() -> %d\n", symbol);
	Cursor cursor = tree.node(symbol);
	if (cursor.is_null()) {
		// Symbol hasn't been transmitted, send a NYT, then the symbol
		encode_and_put(tree.nyt());
		this->put_plain(symbol);
	} else {
		encode_and_put(cursor);
	}
}

template<typename type>
void AdaptiveHuffmanEncoder<type>::encode_and_put(Cursor cursor) {
	if (!cursor.is_null()) {
		encode_and_put(cursor.parent());
		if (!cursor.is_root()) {
			this->put_bit(cursor.side());
		}
	}
}

template<typename symbol_type = char>
class AdaptiveHuffmanDecoder : public Decoder<symbol_type> {
private:
	using Self = AdaptiveHuffmanDecoder;

public:
	using Symbol = symbol_type;

	using Base = Decoder<Symbol>;
	using ICharStream   = typename Base::ICharStream;
	using OSymbolStream = typename Base::OSymbolStream;

	using Tree = AdaptiveHuffmanTree<Symbol>;
	using InternalSymbol = typename Tree::InternalSymbol;
	using Cursor = typename Tree::Cursor;

	using Base::BUFFER_SIZE;
	using Base::BUFFER_BITWIDTH;

private:
	Tree tree;

public:
	AdaptiveHuffmanDecoder(ICharStream & is) : Base(is) {
		// do nothing
	}

	Symbol get();

private:
	// Get a symbol
	InternalSymbol get_symbol();

private:
	AdaptiveHuffmanDecoder(Self const &);
	Self & operator=(Self const &);
}; // class AdaptiveHuffmanDecoder

template<typename type>
auto AdaptiveHuffmanDecoder<type>::get() -> Symbol {
	InternalSymbol internal_symbol = get_symbol();
	Symbol symbol = (internal_symbol == Tree::NYT_SYMBOL) ? this->get_plain() : static_cast<Symbol>(internal_symbol);
	tree << symbol;
	--this->symbol_count;
	return symbol;
}

template<typename type>
auto AdaptiveHuffmanDecoder<type>::get_symbol() -> InternalSymbol {
	Cursor cursor = tree.root();
	for (; !cursor.is_null() && cursor.symbol() == Tree::INTERNAL_SYMBOL; cursor.down(this->get_bit())) {}
	return cursor.symbol();
}

template<typename symbol_type = char>
class AdaptiveHuffmanCodec : public Codec<symbol_type, AdaptiveHuffmanEncoder<symbol_type>, AdaptiveHuffmanDecoder<symbol_type> > {
private:
	using Self = AdaptiveHuffmanCodec;

public:
	using Symbol = symbol_type;

	using Encoder = AdaptiveHuffmanEncoder<Symbol>;
	using Decoder = AdaptiveHuffmanDecoder<Symbol>;
	using Base = Codec<Symbol, Encoder, Decoder>;

	using ICharStream = typename Base::ICharStream;
	using OCharStream = typename Base::OCharStream;

	using ISymbolStream = typename Base::ISymbolStream;
	using OSymbolStream = typename Base::OSymbolStream;

	using ICharFileStream = typename Base::ICharFileStream;
	using OCharFileStream = typename Base::OCharFileStream;

	using ISymbolFileStream = typename Base::ISymbolFileStream;
	using OSymbolFileStream = typename Base::OSymbolFileStream;

	using Base::SYMBOL_BITWIDTH;
	using Base::SYMBOL_NUM;
	using Base::BUFFER_SIZE;
	using Base::BUFFER_BITWIDTH;
}; // class AdaptiveHuffmanCodec

#endif // __ADAPTIVE_HUFFMAN_CODEC_HPP__
