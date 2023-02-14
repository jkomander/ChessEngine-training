#include<cassert>
#include<filesystem>
#include<fstream>
#include<iostream>
#include<string>

struct Position {

};

struct PGN_Converter {
	std::filesystem::path pgn;
	std::filesystem::path trainingData;
	std::vector<char>buffer;
	Position position;
	bool isHeader;
	int8_t gameResult;

	PGN_Converter(
		std::filesystem::path pgn,
		std::filesystem::path training_data
	) :
		pgn(pgn), trainingData(trainingData) {}

	void convert() {
		std::ifstream is(pgn);
		assert(is.is_open());
		std::string line;
		isHeader = true;
		
		size_t i = 0;
		while (std::getline(is, line)) {
			processLine(line);
			
			if (++i == 200)
				break;
		}
	}

	void processLine(std::string_view line) {
		if (!line.size()) {
			isHeader = !isHeader;
			return;
		}

		if (isHeader) {
			assert(line[0] == '[' && line[line.size()-1] == ']');
			if (line.size() >= 14 && line.substr(1, 6) == "Result") {
				std::cout << line << std::endl;
				char c = line[11];
				if (c == '1') gameResult = -1;
				else if (c == '0') gameResult = 1;
				else gameResult = 0;
				std::cout << +gameResult << std::endl;
			}
		}

		else {

		}
	}
};

int main(int argc, char* argv[]) {
	/*std::filesystem::path pgn = argv[1];
	std::filesystem::path trainingData = argv[2];*/

	std::filesystem::path pgn = "C:\\Users\\jacki\\Documents\\C++\\PGN-converter\\out.pgn";
	std::filesystem::path trainingData = "out.td";

	PGN_Converter converter(pgn, trainingData);
	converter.convert();
}