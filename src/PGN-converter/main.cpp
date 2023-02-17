#include"pgn_converter.h"

int main(int argc, char* argv[]) {
	using namespace chess;
	attacks::init();
	/*std::filesystem::path pgn = argv[1];
	std::filesystem::path trainingData = argv[2];*/

	std::filesystem::path pgn = std::filesystem::current_path().append("out.pgn");
	std::filesystem::path trainingData = "out.td";

	PGN_Converter converter(pgn, trainingData);
	converter.convert();
}