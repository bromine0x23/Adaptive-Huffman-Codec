# Adaptive Huffman Codec

## 使用方式
头文件
``` C++
#include "adaptive_huffman_codec.hpp"
```

指定文件名
``` C++
AdaptiveHuffmanCodec<symbol_type>::encode(src_file_name, dest_file_name); // 编码
AdaptiveHuffmanCodec<symbol_type>::decode(src_file_name, dest_file_name); // 解码
```

使用给定的流
``` C++
AdaptiveHuffmanCodec<symbol_type>::encode(src_stream, dest_stream); // 编码
AdaptiveHuffmanCodec<symbol_type>::decode(src_stream, dest_stream); // 解码
```

参见 [encoder.cpp](https://github.com/bromine0x23/Adaptive-Huffman-Codec/blob/master/encoder.cpp) 和 [decoder.cpp](https://github.com/bromine0x23/Adaptive-Huffman-Codec/blob/master/decoder.cpp)