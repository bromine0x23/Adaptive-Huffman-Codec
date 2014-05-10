# Adaptive Huffman Codec

## 使用方式
指定文件名
```C++
	AdaptiveHuffmanCodec<>::encode_to(src_file_name, dest_file_name); // 编码
	AdaptiveHuffmanCodec<>::decode_from(src_file_name, dest_file_name); // 解码
```

使用给定的流
```C++
	AdaptiveHuffmanCodec<>::encode_to(src_stream, dest_stream); // 编码
	AdaptiveHuffmanCodec<>::decode_from(src_stream, dest_stream); // 解码
```