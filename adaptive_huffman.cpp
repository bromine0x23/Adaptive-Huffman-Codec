#ifndef __ADAPTIVE_HUFFMAN_CPP__
#define __ADAPTIVE_HUFFMAN_CPP__

#include "adaptive_huffman.hpp"

#include<cstdio>
#include<iostream>

AdaptiveHuffmanTreeBase::AdaptiveHuffmanTreeBase() : free_linkers(NULL) {
	// do nothing
}

AdaptiveHuffmanTreeBase::~AdaptiveHuffmanTreeBase() {
	for (LinkerPointer linker = free_linkers; linker != NULL;) {
		LinkerPointer old_linker = linker;
		linker = LinkerPointer(*linker);
		delete old_linker;
	}
}

inline AdaptiveHuffmanTreeBase::LinkerPointer AdaptiveHuffmanTreeBase::get_linker(Linker init_value) {
	if (free_linkers != NULL) {
		LinkerPointer linker = free_linkers;
		free_linkers = LinkerPointer(*linker);
		*linker = init_value;
		return linker;
	} else {
		return new Linker(init_value);
	}
}

inline void AdaptiveHuffmanTreeBase::free_linker(LinkerPointer linker) {
	*linker = Linker(free_linkers);
	free_linkers = linker;
}

template<typename type>
AdaptiveHuffmanTree<type>::AdaptiveHuffmanTree() {
	for (int i = 0; i <= SYMBOL_NUM; ++i) {
		location[i] = NULL;
	}
	tree_root = list_head = location[NYT_SYMBOL] = get_node(NYT_SYMBOL);
	list_head->block_head = get_linker(list_head);
}

template<typename type>
AdaptiveHuffmanTree<type>::~AdaptiveHuffmanTree() {
	for (NodePointer node = list_head; ;) {
		NodePointer old_node = node;
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
}

template<typename type>
inline void AdaptiveHuffmanTree<type>::update(Symbol symbol) {
	if (location[symbol] == NULL) {
		add_symbol(symbol);
	}
	increment(location[symbol]);
}

template<typename type>
void AdaptiveHuffmanTree<type>::add_symbol(Symbol symbol) {
	assert(list_head->symbol == NYT_SYMBOL);

	NodePointer symbol_node = get_node(symbol);
	push_head(symbol_node);

	NodePointer new_nyt_node = get_node(NYT_SYMBOL);
	push_head(new_nyt_node);

	NodePointer old_nyt_node = location[NYT_SYMBOL];
	old_nyt_node->symbol = INTERNAL_SYMBOL;

	new_nyt_node->parent = symbol_node->parent = old_nyt_node;
	old_nyt_node->left = new_nyt_node;
	old_nyt_node->right = symbol_node;

	location[symbol] = symbol_node;
	location[NYT_SYMBOL] = new_nyt_node;
}

template<typename type>
void AdaptiveHuffmanTree<type>::increment(NodePointer node) {
	assert(node != NULL);

	if (node->next != NULL && node->next->weight == node->weight) {
		Linker head = *node->block_head;
		assert(head != node);
		assert(head->parent != node);
		assert(head->weight == node->weight);
		if (head != node->parent) {
			swap_in_tree(head, node);
		} else {
			assert(node->next == head);
		}
		swap_in_list(head, node);
	}

	if (node->prev != NULL && node->prev->weight == node->weight) {
		*node->block_head = node->prev;
	} else {
		free_linker(node->block_head);
		node->block_head = NULL;
	}

	++(node->weight);

	if (node->next && node->next->weight == node->weight) {
		node->block_head = node->next->block_head;
	} else {
		node->block_head = get_linker(node);
	}

	if (node->parent != NULL) {
		increment(node->parent);
		if (node->prev == node->parent) {
			swap_in_list(node, node->parent);
			if (*node->block_head == node) {
				*node->block_head = node->parent;
			}
		}
	}
}

template<typename type>
inline typename AdaptiveHuffmanTree<type>::NodePointer AdaptiveHuffmanTree<type>::get_node(Symbol symbol) {
	NodePointer node = new Node;
	node->symbol = symbol;
	node->weight = 0;
	node->parent = node->left = node->right = NULL;
	node->block_head = NULL;
	node->next = node->prev = NULL;
	return node;
}

template<typename type>
inline void AdaptiveHuffmanTree<type>::push_head(NodePointer node) {
	node->next = list_head;
	list_head->prev = node;
	node->block_head = list_head->block_head;
	list_head = node;
}

template<typename type>
void AdaptiveHuffmanTree<type>::swap_in_tree(NodePointer node1, NodePointer node2) {
	assert(node1->symbol != NYT_SYMBOL && node2->symbol != NYT_SYMBOL);

	NodePointer node1_parent = node1->parent;
	NodePointer node2_parent = node2->parent;

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

	if (node2_parent) {
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
void AdaptiveHuffmanTree<type>::swap_in_list(NodePointer node1, NodePointer node2) {
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
inline void AdaptiveHuffmanTree<type>::dump_tree(std::basic_ostream<Char> & os) const {
	dump_tree(os, tree_root);
	os << std::endl;
}

template<typename type>
void AdaptiveHuffmanTree<type>::dump_tree(std::basic_ostream<Char> & os, ConstantNodePointer node) const {
	for (; node != NULL; node = node->right) {
		switch (node->symbol) {
		case INTERNAL_SYMBOL:
			os << '.';
			break;
		case NYT_SYMBOL:
			os << '@';
			break;
		case '\n':
			os << '%';
			break;
		default:
			os << node->symbol << ' ';
			break;
		}

		if (node->left != NULL) {
			assert(node->left->parent == node);
			dump_tree(os, node->left);
		}

		if (node->right != NULL) {
			assert(node->right->parent == node);
		}
	}
}

template<typename type>
void AdaptiveHuffmanTree<type>::dump_list(std::basic_ostream<Char> & os) const {
	for (ConstantNodePointer node = list_head; node != NULL; node = node->next) {
		switch (node->symbol) {
		case INTERNAL_SYMBOL:
			os << '.';
			break;
		case NYT_SYMBOL:
			os << '@';
			break;
		case '\n':
			os << '%';
			break;
		default:
			os << node->symbol;
			break;
		}
		os << '(' << node->weight << ')';
	}
	os << std::endl;
}

template<typename type>
AdaptiveHuffmanEncoder<type>::AdaptiveHuffmanEncoder(OStream & os) : ostream(os), symbol_count(0) {
	clear_buffer();
}

template<typename type>
AdaptiveHuffmanEncoder<type> & AdaptiveHuffmanEncoder<type>::put(Symbol symbol) {
	++symbol_count;
	put_symbol(symbol);
	tree.update(symbol);
	return *this;
}

template<typename type>
AdaptiveHuffmanEncoder<type> & AdaptiveHuffmanEncoder<type>::put_tail() {
	flush();
	for (int n = BitwidthOf<Counter>::value - 1; n >= 0; --n) {
		put_bit((symbol_count >> n) & 0x1);
	}
	flush();
	return *this;
}

template<typename type>
inline void AdaptiveHuffmanEncoder<type>::flush() {
	ostream.write(write_buffer, (bit_count + BIT_PER_CHAR - 1) / BIT_PER_CHAR);
	clear_buffer();
}

template<typename type>
inline AdaptiveHuffmanEncoder<type> & AdaptiveHuffmanEncoder<type>::operator<<(Symbol symbol) {
	return put(symbol);
}

template<typename type>
inline typename AdaptiveHuffmanEncoder<type>::Counter AdaptiveHuffmanEncoder<type>::count() const {
	return symbol_count;
}

template<typename type>
inline void AdaptiveHuffmanEncoder<type>::clear_buffer() {
	std::uninitialized_fill_n(write_buffer, BUFFER_SIZE, 0);
	bit_count = 0;
}

template<typename type>
void AdaptiveHuffmanEncoder<type>::put_symbol(Symbol symbol) {
	Cursor cursor = tree.node(symbol);
	if (cursor.is_null()) { // Node hasn't been transmitted, send a NYT, then the symbol
		put_symbol(Tree::NYT_SYMBOL);
		for (int i = Tree::SYMBOL_BIT - 1; i >= 0; --i) {
			put_bit((symbol >> i) & 0x1);
		}
	} else {
		encode(cursor);
	}
}

template<typename type>
void AdaptiveHuffmanEncoder<type>::encode(Cursor cursor) {
	if (!cursor.is_null()) {
		encode(cursor.parent());
		if (!cursor.is_root()) {
			if (cursor.is_left_child()) {
				put_bit(0);
			} else {
				assert(cursor.is_right_child());
				put_bit(1);
			}
		}
	}
}

template<typename type>
void AdaptiveHuffmanEncoder<type>::put_bit(Bit bit) {
	assert(bit == 0 || bit == 1);
	if (bit_count == BUFFER_SIZE * BIT_PER_CHAR) {
		ostream.write(write_buffer, BUFFER_SIZE);
		clear_buffer();
	}
	write_buffer[bit_count / BIT_PER_CHAR] |= bit << (BIT_PER_CHAR - (bit_count % BIT_PER_CHAR) - 1);
	++bit_count;
}

template<typename type>
AdaptiveHuffmanDecoder<type>::AdaptiveHuffmanDecoder(IStream & is) : istream(is), bit_count(BUFFER_SIZE * BIT_PER_CHAR) {
	int const length = sizeof(Counter);
	typename IStream::pos_type pos = istream.tellg();
	Char buffer[length];
	istream.seekg(-length, std::ios_base::end);
	istream.read(buffer, length);
	symbol_count = 0;
	for (int i = 0; i < length; ++i) {
		symbol_count = symbol_count << BIT_PER_CHAR | ((CHAR_NUM ^ buffer[i]) & (CHAR_NUM - 1));
	}
	istream.seekg(pos);
	memset(read_buffer, 0, BUFFER_SIZE);
}

template<typename type>
inline typename AdaptiveHuffmanDecoder<type>::Counter AdaptiveHuffmanDecoder<type>::count() const {
	return symbol_count;
}

template<typename type>
typename AdaptiveHuffmanDecoder<type>::Symbol AdaptiveHuffmanDecoder<type>::get() {
	Symbol symbol = get_symbol();
	if (symbol == Tree::NYT_SYMBOL) {
		symbol = 0;
		for (int i = 0; i < Tree::SYMBOL_BIT; ++i) {
			symbol = symbol << 1 | get_bit();
		}
	}
	tree.update(symbol);
	--symbol_count;
	return symbol;
}

template<typename type>
typename AdaptiveHuffmanDecoder<type>::Symbol AdaptiveHuffmanDecoder<type>::get_symbol() {
	Cursor cursor = tree.root();
	for (; !cursor.is_null() && cursor.symbol() == Tree::INTERNAL_SYMBOL;) {
		if (get_bit() == 0) {
			cursor.down_left();
		} else {
			cursor.down_right();
		}
	}
	return cursor.symbol();
}

template<typename type>
typename AdaptiveHuffmanDecoder<type>::Bit AdaptiveHuffmanDecoder<type>::get_bit() {
	if (bit_count == BUFFER_SIZE * BIT_PER_CHAR) {
		istream.read(read_buffer, BUFFER_SIZE);
		bit_count = 0;
	}
	Bit bit = (read_buffer[bit_count / BIT_PER_CHAR] >> (BIT_PER_CHAR - (bit_count % BIT_PER_CHAR) - 1)) & 0x1;
	++bit_count;
	return bit;
}

template<typename type>
void AdaptiveHuffmanCodec<type>::encode_to(IStream & istream, OStream & ostream) {
	Encoder encoder(ostream);
	for (Char ch;;) {
		if (!istream.get(ch).good()) {
			break;
		}
		if (ch >= 0) {
			encoder.put(ch);
		} else {
			encoder.put(CHAR_NUM + ch);
		}

	}
	encoder.put_tail();
}

template<typename type>
void AdaptiveHuffmanCodec<type>::decode_from(OStream & ostream, IStream & istream) {
	Decoder decoder(istream);
	for (; decoder.count() > 0;) {
		Symbol symbol = decoder.get();
		if (symbol < CHAR_NUM_HALF) {
			ostream.put(Char(symbol));
		} else {
			ostream.put(Char(symbol - CHAR_NUM));
		}

	}
}

template<typename type>
inline void AdaptiveHuffmanCodec<type>::encode_to(IStream & istream, char const * file_name) {
	OFileStream fout(file_name, std::ios::binary);
	encode_to(istream, fout);
}

template<typename type>
inline void AdaptiveHuffmanCodec<type>::decode_from(OStream & ostream, char const * file_name) {
	IFileStream fin(file_name, std::ios::binary);
	decode_from(ostream, fin);
}

template<typename type>
inline void AdaptiveHuffmanCodec<type>::encode_to(char const * input_file, char const * output_file) {
	IFileStream fin(input_file, std::ios::binary);
	OFileStream fout(output_file, std::ios::binary);
	encode_to(fin, fout);
}

template<typename type>
inline void AdaptiveHuffmanCodec<type>::decode_from(char const * input_file, char const * output_file) {
	IFileStream fin(input_file, std::ios::binary);
	OFileStream fout(output_file, std::ios::binary);
	decode_from(fout, fin);
}

#endif // __ADAPTIVE_HUFFMAN_CPP__
