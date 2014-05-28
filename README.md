# Adaptive Huffman Codec

## ʹ�÷�ʽ
ͷ�ļ�
``` C++
#include "adaptive_huffman_codec.hpp"
```

ָ���ļ���
``` C++
AdaptiveHuffmanCodec<symbol_type>::encode(src_file_name, dest_file_name); // ����
AdaptiveHuffmanCodec<symbol_type>::decode(src_file_name, dest_file_name); // ����
```

ʹ�ø�������
``` C++
AdaptiveHuffmanCodec<symbol_type>::encode(src_stream, dest_stream); // ����
AdaptiveHuffmanCodec<symbol_type>::decode(src_stream, dest_stream); // ����
```

�μ� [encoder.cpp](https://github.com/bromine0x23/Adaptive-Huffman-Codec/blob/master/encoder.cpp) �� [decoder.cpp](https://github.com/bromine0x23/Adaptive-Huffman-Codec/blob/master/decoder.cpp)