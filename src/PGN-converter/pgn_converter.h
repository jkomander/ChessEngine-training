#pragma once

#include<cassert>
#include<filesystem>
#include<fstream>
#include<iostream>
#include<string>

#include"pgn_position.h"

namespace chess {

	struct PGN_Converter {
		std::filesystem::path pgn;
		std::filesystem::path trainingData;
		std::vector<char>buffer;
		PGN_Position position;
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

			size_t i = 0;
			while (std::getline(is, line)) {
				processLine(line);

				if (++i == 20)
					break;
			}
		}

		void processLine(std::string_view line) {
			if (!line.size()) return;

			if (line[0] == '[') {
				assert(line[line.size()-1] == ']');
				if (line.size() >= 14 && line.substr(1, 6) == "Result") {
					// Read game result
					char c = line[11];
					if (c == '1') gameResult = -1;
					else if (c == '0') gameResult = 1;
					else gameResult = 0;
				}
			}

			else {
				size_t idx = 0;
				std::cout << line << std::endl;
				for (; idx < line.size();) {
					size_t spaceIdx = line.find_first_of(' ', idx);
					size_t len = spaceIdx == std::string_view::npos ? line.size() - idx : spaceIdx - idx;
					std::cout << line.substr(idx, len) << std::endl;
					idx += len + 1;
				}
			}
		}
	};

} // namespace chess