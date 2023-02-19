#include<chrono>

#include"pgn_converter.h"

int main(int argc, char* argv[]) {
	auto t0 = std::chrono::high_resolution_clock::now();

	chess::attacks::init();
	chess::pgn::init();

	std::filesystem::path pgn = argv[1];
	std::filesystem::path trainingData = argv[2];

	std::cout << "Converting " << pgn << " to " << trainingData << "." << std::endl;

	chess::pgn::Converter converter(pgn, trainingData);
	converter.convert();

	auto t1 = std::chrono::high_resolution_clock::now();
	std::cout << "Elapsed time: " << (t1-t0).count() * 1e-9 << std::endl;
}