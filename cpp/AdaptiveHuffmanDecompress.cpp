/* 
 * Decompression application using adaptive Huffman coding
 * 
 * Usage: AdaptiveHuffmanDecompress InputFile OutputFile
 * This decompresses files generated by the "AdaptiveHuffmanCompress" application.
 * 
 * Copyright (c) Project Nayuki
 * 
 * https://www.nayuki.io/page/reference-huffman-coding
 * https://github.com/nayuki/Reference-Huffman-coding
 */

#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <vector>
#include "BitIoStream.hpp"
#include "FrequencyTable.hpp"
#include "HuffmanCoder.hpp"

using std::uint32_t;


static bool isPowerOf2(uint32_t x);


int main(int argc, char *argv[]) {
	// Handle command line arguments
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " InputFile OutputFile" << std::endl;
		return EXIT_FAILURE;
	}
	const char *inputFile  = argv[1];
	const char *outputFile = argv[2];
	
	// Perform file decompression
	std::ifstream in(inputFile, std::ios::binary);
	std::ofstream out(outputFile, std::ios::binary);
	BitInputStream bin(in);
	try {
		
		std::vector<uint32_t> initFreqs(257, 1);
		FrequencyTable freqs(initFreqs);
		HuffmanDecoder dec(bin);
		CodeTree tree = freqs.buildCodeTree();  // Use same algorithm as the compressor
		dec.codeTree = &tree;
		uint32_t count = 0;  // Number of bytes written to the output file
		while (true) {
			// Decode and write one byte
			uint32_t symbol = dec.read();
			if (symbol == 256)  // EOF symbol
				break;
			int b = static_cast<int>(symbol);
			if (std::numeric_limits<char>::is_signed)
				b -= (b >> 7) << 8;
			out.put(static_cast<char>(b));
			count++;
			
			// Update the frequency table and possibly the code tree
			freqs.increment(symbol);
			if ((count < 262144 && isPowerOf2(count)) || count % 262144 == 0)  // Update code tree
				tree = freqs.buildCodeTree();
			if (count % 262144 == 0)  // Reset frequency table
				freqs = FrequencyTable(initFreqs);
		}
		return EXIT_SUCCESS;
		
	} catch (const char *msg) {
		std::cerr << msg << std::endl;
		return EXIT_FAILURE;
	}
}


static bool isPowerOf2(uint32_t x) {
	return x > 0 && (x & (x - 1)) == 0;
}
