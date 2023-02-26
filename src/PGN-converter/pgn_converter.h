#pragma once

#include<cassert>
#include<cstring>
#include<filesystem>
#include<fstream>
#include<iostream>
#include<string>
#include<vector>

#include"pgn_position.h"

namespace chess {

	namespace pgn {

		struct Converter {
			std::filesystem::path pgn;
			std::filesystem::path trainingData;
			std::vector<char>buffer;
			Position position;
			std::string fen;
			int8_t gameResult;
			Score score;
			bool isComment;
			bool foundFEN;
			bool isTagPair;

			Converter(
				std::filesystem::path pgn,
				std::filesystem::path trainingData
			) :
				pgn(pgn), trainingData(trainingData) {}

			void convert() {
				std::ifstream is(pgn);
				assert(is.is_open());
				std::string line;
				buffer = {};
				isComment = false;
				isTagPair = true;
				foundFEN = false;

				size_t lineCount{};
				while (std::getline(is, line)) {
					processLine(line);
					++lineCount;
				}

				std::ofstream os(trainingData, std::ios::out | std::ios::binary);
				os.write(buffer.data(), buffer.size());
			}

			void processLine(std::string_view line) {
				if (!line.size()) {
					if (isTagPair) {
						if (!foundFEN)
							position = Position::startPosition();
						isTagPair = false;
						foundFEN = false;
					}
					return;
				}

				// tag pairs
				if (line[0] == '[') {
					assert(line[line.size()-1] == ']');
					if (line.size() >= 14 && line.substr(1, 6) == "Result") {
						// read game result
						char c = line[11];
						if (c == '1') gameResult = -1;
						else if (c == '0') gameResult = 1;
						else gameResult = 0;
					}

					else if (line.size() >= 6 && line.substr(1, 3) == "FEN") {
						foundFEN = true;
						position = { line.substr(6, line.size()-3) };
					}

					isTagPair = true;
				}

				// movetext
				else {
					size_t idx = 0;
					// std::cout << line << std::endl;
					for (; idx < line.size();) {
						size_t spaceIdx = line.find_first_of(' ', idx);
						size_t len = spaceIdx == std::string_view::npos ? line.size() - idx : spaceIdx - idx;

						if (line[idx] == '{') isComment = true;

						// comment, read score
						if (isComment) {
							bool foundScore = false;

							if (line[idx+1] == '+') {
								foundScore = true;
								uint8_t slashIdx = line.find_first_of('/', idx);
								assert(slashIdx != std::string_view::npos);

								if (line[idx+2] == 'M')
									score = MATE_SCORE - std::stoi(std::string(line.substr(idx+3, slashIdx-idx-3)));
								else
									score = 100 * ::atof(std::string(line.substr(idx+2, slashIdx-idx-2)).c_str());
							}

							else if (line[idx+1] == '-') {
								foundScore = true;
								uint8_t slashIdx = line.find_first_of('/', idx);
								assert(slashIdx != std::string_view::npos);

								if (line[idx+2] == 'M')
									score = -MATE_SCORE + std::stoi(std::string(line.substr(idx+3, slashIdx-idx-3)));
								else
									score = -100 * ::atof(std::string(line.substr(idx+2, slashIdx-idx-2)).c_str());
							}

							if (line[idx+len-1] == '}')
								isComment = false;

							// store extended FEN in buffer
							if (foundScore) {
								assert(fen.size() <= 255);
								uint8_t fenSize = fen.size();
								int8_t relativeGameResult = position.stm ? gameResult : -gameResult;

								size_t offset = buffer.size();
								buffer.resize(offset +
									sizeof(uint8_t) + fenSize + sizeof(Score) + sizeof(int8_t)
								);

								buffer[offset++] = fenSize;
								std::memcpy(&buffer[offset], fen.data(), fenSize);
								offset += fenSize;
								std::memcpy(&buffer[offset], &score, sizeof(Score));
								offset += sizeof(Score);
								buffer[offset] = relativeGameResult;
							}
						}

						else if (line[idx+len-1] != '.' &&
							line.substr(idx, len) != "1-0" &&
							line.substr(idx, len) != "0-1"&&
							line.substr(idx, len) != "1/2-1/2")
						{
							fen = position.fen();
							position.applyMove(line.substr(idx, len));
						}

						idx += len + 1;
					}
				}
			}
		};

	} // namespace pgn

} // namespace chess