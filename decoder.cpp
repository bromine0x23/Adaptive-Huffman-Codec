#include "adaptive_huffman.cpp"

#include <iostream>

void usage() {
	std::cerr
		<< "Usage:\n"
		<< "encoder src dest\n";
}

int main(int argc, char** argv) {
	if (argc != 3) {
		usage();
		return -1;
	}

	AdaptiveHuffmanCodec<>::decode_from(argv[1], argv[2]);

	return 0;
}
