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

	static InternalSymbol to_internal(Symbol symbol) {
		return static_cast<UInt64>(static_cast<typename Unsigned<Symbol>::Type>(symbol));
	}

	static Symbol to_external(InternalSymbol symbol) {
		return static_cast<Symbol>(static_cast<typename Unsigned<Symbol>::Type>(symbol));
	}

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
	static Size const SYMBOL_BIT = BitwidthOf<Symbol>::value;
	static Size const SYMBOL_NUM = 1ULL << SYMBOL_BIT;
	static InternalSymbol const NYT_SYMBOL = SYMBOL_NUM; // id of not yet transmitted
	static InternalSymbol const INTERNAL = SYMBOL_NUM + 1; // id of internal node

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
			return node == nullptr;
		}

		bool is_root() const {
			return node->parent == nullptr;
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

private:
	Linker * free_linkers;
	Node * tree_root;
	Linker list_head;
	Linker location[SYMBOL_NUM + 1];

public:
	AdaptiveHuffmanTree() : free_linkers(nullptr) {
		for (int i = 0; i <= SYMBOL_NUM; ++i) {
			location[i] = nullptr;
		}
		tree_root = list_head = location[NYT_SYMBOL] = get_node(NYT_SYMBOL);
		list_head->block_head = get_linker(list_head);
	}


	virtual ~AdaptiveHuffmanTree();

	Cursor root() const {
		return Cursor(tree_root);
	}

	Cursor operator[](InternalSymbol symbol) const {
		return Cursor(location[symbol]);
	}

	Cursor operator[](Symbol symbol) const {
		return (*this)[to_internal(symbol)];
	}

	Cursor nyt() const {
		return (*this)[NYT_SYMBOL];
	}

	Self & operator<<(Symbol symbol) {
		InternalSymbol internal_symbol = to_internal(symbol);
		if (location[internal_symbol] == nullptr) {
			new_symbol(internal_symbol);
		}
		increse_weight(location[internal_symbol]);
		return *this;
	}

private:
	void new_symbol(InternalSymbol symbol);

	// Do the increments
	void increse_weight(Node * node);

	Node * get_node(InternalSymbol symbol) {
		Node * node = new Node;
		node->symbol = symbol;
		node->weight = 0;
		node->parent = node->left = node->right = nullptr;
		node->block_head = nullptr;
		node->next = node->prev = nullptr;
		return node;
	}

	Linker * get_linker(Linker value) {
		Linker * linker;
		if (free_linkers != nullptr) {
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
AdaptiveHuffmanTree<type>::~AdaptiveHuffmanTree() {
	for (Node * node = list_head;;) {
		Node * old_node = node;
		node = node->next;
		if (node != nullptr) {
			if (old_node->weight != node->weight) {
				delete old_node->block_head;
			}
			delete old_node;
		} else {
			break;
		}
	}
	for (Linker * linker = free_linkers; linker != nullptr;) {
		Linker * old_linker = linker;
		linker = reinterpret_cast<Linker *>(*linker);
		delete old_linker;
	}
}

template<typename type>
void AdaptiveHuffmanTree<type>::new_symbol(InternalSymbol symbol) {
	assert(list_head->symbol == NYT_SYMBOL);

	Node * symbol_node = get_node(symbol);
	push_head(symbol_node);

	Node * new_nyt_node = get_node(NYT_SYMBOL);
	push_head(new_nyt_node);

	Node * old_nyt_node = location[NYT_SYMBOL];
	assert(old_nyt_node != nullptr && old_nyt_node->left == nullptr && old_nyt_node->right == nullptr);
	old_nyt_node->symbol = INTERNAL;

	new_nyt_node->parent = symbol_node->parent = old_nyt_node;
	old_nyt_node->left = new_nyt_node;
	old_nyt_node->right = symbol_node;

	location[symbol] = symbol_node;
	location[NYT_SYMBOL] = new_nyt_node;
}

template<typename type>
void AdaptiveHuffmanTree<type>::increse_weight(Node * node) {
	assert(node != nullptr);

	if (node->next != nullptr && node->next->weight == node->weight) {
		Linker head = *node->block_head;
		assert(head != node && head->parent != node && head->weight == node->weight);
		if (head != node->parent) {
			swap_in_tree(head, node);
		} else {
			assert(node->next == head);
		}
		swap_in_list(head, node);
	}

	if (node->prev != nullptr && node->weight == node->prev->weight) {
		*node->block_head = node->prev;
	} else {
		put_linker(node->block_head);
		node->block_head = nullptr;
	}

	++node->weight;

	if (node->next != nullptr && node->weight == node->next->weight) {
		node->block_head = node->next->block_head;
	} else {
		node->block_head = get_linker(node);
	}

	if (node->parent != nullptr) {
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

	if (node1_parent != nullptr) {
		if (node1_parent->left == node1) {
			node1_parent->left = node2;
		} else {
			assert(node1_parent->right == node1);
			node1_parent->right = node2;
		}
	} else {
		tree_root = node2;
	}

	if (node2_parent != nullptr) {
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

	if (node1->next != nullptr) {
		node1->next->prev = node1;
	}
	if (node2->next != nullptr) {
		node2->next->prev = node2;
	}

	if (node1->prev != nullptr) {
		node1->prev->next = node1;
	}
	if (node2->prev != nullptr) {
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
		assert(node->next == nullpter || node->weight <= node->next->weight);
		assert(node->block_head != nullpter && *(node->block_head) != nullpter && (*node->block_head)->weight == node->weight);
		if (node->next != nullpter) {
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
	for (ConstantNodePointer node = list_head; node != nullpter; node = node->next) {
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
	for (; node != nullpter; node = node->right) {
		std::cout << '[' << node->symbol << ']';
		if (node->left != nullpter) {
			assert(node->left->parent == node);
			dump_tree(node->left);
		}

		if (node->right != nullpter) {
			assert(node->right->parent == node);
		}
	}
}
#endif // NDEBUG

#include "codec.hpp"

template<typename symbol_type = Byte>
class AdaptiveHuffmanEncoder : public Encoder<symbol_type> {
private:
	using Self = AdaptiveHuffmanEncoder;

public:
	using Symbol = symbol_type;

	using Base = Encoder<Symbol>;

private:
	using Tree = AdaptiveHuffmanTree<Symbol>;

	Tree tree;

public:
	AdaptiveHuffmanEncoder(typename Base::OStream & os) : Base(os) {
		// do nothing
	}

	Self & put(Symbol symbol) {
		put_symbol(symbol);
		tree << symbol;
		++Base::symbol_count;
		return *this;
	}

private:
	// Send a symbol
	void put_symbol(Symbol symbol) {
		auto cursor = tree[symbol];
		if (cursor.is_null()) {
			// Symbol hasn't been transmitted, send a NYT, then the symbol
			encode_and_put(tree.nyt());
			Base::put_plain(symbol);
		} else {
			encode_and_put(cursor);
		}
	}

	// Encode symbol and send code
	void encode_and_put(typename Tree::Cursor cursor) {
		if (!cursor.is_null()) {
			encode_and_put(cursor.parent());
			if (!cursor.is_root()) {
				Base::put_bit(cursor.side());
			}
		}
	}

private:
	AdaptiveHuffmanEncoder(Self const &) = delete;
	Self & operator=(Self const &) = delete;
}; // class AdaptiveHuffmanEncoder

template<typename symbol_type = Byte>
class AdaptiveHuffmanDecoder : public Decoder<symbol_type> {
private:
	using Self = AdaptiveHuffmanDecoder;

public:
	using Symbol = symbol_type;

	using Base = Decoder<Symbol>;

private:
	using Tree = AdaptiveHuffmanTree<Symbol>;

	Tree tree;

public:
	AdaptiveHuffmanDecoder(typename Base::IStream & is) : Base(is) {
		// do nothing
	}

	Symbol get() {
		auto symbol = get_symbol();
		tree << symbol;
		--Base::symbol_count;
		return symbol;
	}

private:
	// Get a symbol
	Symbol get_symbol() {
		auto cursor = tree.root();
		for (; !cursor.is_null() && cursor.symbol() == Tree::INTERNAL; ) {
			cursor.down(Base::get_bit());
		}
		if (cursor.symbol() == Tree::NYT_SYMBOL) {
			return Base::get_plain();
		} else {
			return Tree::to_external(cursor.symbol());
		}
	}

private:
	AdaptiveHuffmanDecoder(Self const &) = delete;
	Self & operator=(Self const &) = delete;
}; // class AdaptiveHuffmanDecoder

template<typename symbol_type = Byte>
class AdaptiveHuffmanCodec : public Codec<symbol_type, AdaptiveHuffmanEncoder<symbol_type>, AdaptiveHuffmanDecoder<symbol_type>> {
private:
	using Self = AdaptiveHuffmanCodec;

public:
	using Symbol = symbol_type;

	using Encoder = AdaptiveHuffmanEncoder<Symbol>;
	using Decoder = AdaptiveHuffmanDecoder<Symbol>;

	using Base = Codec<Symbol, Encoder, Decoder>;

}; // class AdaptiveHuffmanCodec

#endif // __ADAPTIVE_HUFFMAN_CODEC_HPP__
