#pragma once

#include"../chess/attacks.h"

namespace chess {

	namespace pgn {

		struct CastlingRights {
			uint8_t data;

			enum Flags {
				NO_CASTLING,
				WHITE_QUEEN_SIDE = 1,
				WHITE_KING_SIDE = 1 << 1,
				BLACK_QUEEN_SIDE = 1 << 2,
				BLACK_KING_SIDE = 1 << 3,

				QUEEN_SIDE = WHITE_QUEEN_SIDE | BLACK_QUEEN_SIDE,
				KING_SIDE = WHITE_KING_SIDE | BLACK_KING_SIDE,
				WHITE_CASTLING = WHITE_KING_SIDE | WHITE_QUEEN_SIDE,
				BLACK_CASTLING = BLACK_KING_SIDE | BLACK_QUEEN_SIDE,
				ANY_CASTLING = WHITE_CASTLING | BLACK_CASTLING,
			};
		};

		struct FenTableEntry {
			bool setPiece;
			Piece piece;
			int8_t skip;
		};

		inline const static std::string START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
		inline FenTableEntry fenTable[128];
		inline Piece charToPieceType[128];

		void init();

		struct Position {
			Piece board[N_SQUARES];
			Bitboard occupied;
			Bitboard byPieceType[N_PIECE_TYPES];
			Bitboard byColor[N_COLORS];
			Color stm;
			CastlingRights castlingRights;
			Square epSquare;
			uint8_t rule50Cnt;
			uint16_t ply;

			Position() = default;
			Position(std::string_view fen);

			static Position startPosition() {
				return Position(START_FEN);
			}

			friend std::ostream& operator<<(std::ostream& os, const Position& position);

			std::string fen() const;

			void applyMove(std::string_view sv);
			template<PieceType pt> void readMove(std::string_view sv);

			bool canCastle(uint8_t flag) const {
				return castlingRights.data & flag;
			}

			bool canCastle() const {
				return castlingRights.data;
			}

			Piece piece(Square s) const {
				return board[s];
			}

			Bitboard pieces(PieceType pt) const {
				return byPieceType[pt];
			}

			Bitboard pieces(Color c, PieceType pt) const {
				return byColor[c] & byPieceType[pt];
			}

			Bitboard piecesByColor(Color c) const {
				return byColor[c];
			}

			Square kingSquare(Color c) const {
				return pieces(c, KING).LSB();
			}

			void setPiece(Square s, Piece pc);
			void removePiece(Square s);
			void movePiece(Square from, Square to);

			Bitboard pinned() const;
		};

		void init() {
			// From https://github.com/Luecx/CudAD/blob/main/src/position/fenparsing.h
			std::memset(fenTable, 0, 128 * sizeof(FenTableEntry));

			fenTable['P'] = { true, WHITE_PAWN, EAST };
			fenTable['N'] = { true, WHITE_KNIGHT, EAST };
			fenTable['B'] = { true, WHITE_BISHOP, EAST };
			fenTable['R'] = { true, WHITE_ROOK, EAST };
			fenTable['Q'] = { true, WHITE_QUEEN, EAST };
			fenTable['K'] = { true, WHITE_KING, EAST };

			fenTable['p'] = { true, BLACK_PAWN, EAST };
			fenTable['n'] = { true, BLACK_KNIGHT, EAST };
			fenTable['b'] = { true, BLACK_BISHOP, EAST };
			fenTable['r'] = { true, BLACK_ROOK, EAST };
			fenTable['q'] = { true, BLACK_QUEEN, EAST };
			fenTable['k'] = { true, BLACK_KING, EAST };

			fenTable['1'] = { false, {}, EAST };
			fenTable['2'] = { false, {}, 2 * EAST };
			fenTable['3'] = { false, {}, 3 * EAST };
			fenTable['4'] = { false, {}, 4 * EAST };
			fenTable['5'] = { false, {}, 5 * EAST };
			fenTable['6'] = { false, {}, 6 * EAST };
			fenTable['7'] = { false, {}, 7 * EAST };
			fenTable['8'] = { false, {}, 8 * EAST };

			fenTable['/'] = { false, {}, 2 * SOUTH };

			// char to piece
			std::memset(charToPieceType, 0, 128 * sizeof(Piece));
			for (char c = 'a'; c <= 'h'; ++c)
				charToPieceType[c] = PAWN;
			charToPieceType['N'] = KNIGHT;
			charToPieceType['B'] = BISHOP;
			charToPieceType['R'] = ROOK;
			charToPieceType['Q'] = QUEEN;
			charToPieceType['K'] = KING;
			charToPieceType['O'] = KING;
		}

		Position::Position(std::string_view fen) {

			std::memset(this, 0, sizeof(Position));
			uint16_t idx = 0;

			// piece placement
			Square sq = A8;

			for (; idx < fen.size() && fen[idx] != ' '; ++idx) {
				const FenTableEntry& e = fenTable[fen[idx]];
				if (e.setPiece)
					setPiece(sq, e.piece);
				sq += e.skip;
			}

			++idx;

			// side to move
			stm = fen[idx] == 'w' ? WHITE : BLACK;

			idx += 2;

			// castling ability
			for (; idx < fen.size() && fen[idx] != ' '; ++idx) {
				if (fen[idx] == '-') continue;

				switch (fen[idx]) {
				case 'K':
					castlingRights.data |= CastlingRights::WHITE_KING_SIDE;
					break;
				case 'Q':
					castlingRights.data |= CastlingRights::WHITE_QUEEN_SIDE;
					break;
				case 'k':
					castlingRights.data |= CastlingRights::BLACK_KING_SIDE;
					break;
				case 'q':
					castlingRights.data |= CastlingRights::BLACK_QUEEN_SIDE;
					break;
				}
			}

			++idx;

			// en passant target square
			if (fen[idx] != '-') {
				epSquare = square::make(file::CHAR_IDENTIFYERS[idx++], rank::CHAR_IDENTIFYERS[idx++]);
			}

			idx += 2;

			// 50 move rule
			if (fen[idx] != '-') {
				auto len = fen.find_first_of(' ', idx);
				auto numeric = std::stoi(std::string(fen.substr(idx, len)));
				rule50Cnt = numeric;
				idx = len - 1;
			}

			idx += 2;

			// move counter
			if (fen[idx] != '-') {
				auto len = fen.find_first_of(' ', idx);
				auto numeric = std::stoi(std::string(fen.substr(idx, len)));
				ply = 2 * (numeric - 1) + stm;
				idx = len - 1;
			}
		}

		std::string Position::fen() const {
			std::stringstream ss;

			for (Rank r = RANK_8; r >= RANK_1; --r) {
				int emptyCount = 0;
				for (File f = FILE_A; f <= FILE_H; ++f) {
					Piece pc = piece(square::make(f, r));
					if (pc) {
						if (emptyCount)
							ss << emptyCount;
						ss << piece::PIECE_TO_CHAR[pc];
						emptyCount = 0;
					}
					else ++emptyCount;
				}
				if (emptyCount) ss << emptyCount;
				if (r) ss << '/';
			}

			ss << ' ' << (stm ? 'b' : 'w');

			ss << ' ';
			if (canCastle(CastlingRights::WHITE_KING_SIDE)) ss << 'K';
			if (canCastle(CastlingRights::WHITE_QUEEN_SIDE)) ss << 'Q';
			if (canCastle(CastlingRights::BLACK_KING_SIDE)) ss << 'k';
			if (canCastle(CastlingRights::BLACK_QUEEN_SIDE)) ss << 'q';
			if (!canCastle()) ss << '-';
			ss << ' ' << (epSquare ? square::toString(epSquare) : "-");
			ss << ' ' << (int)rule50Cnt;
			ss << ' ' << 1 + (ply - stm) / 2;

			return ss.str();
		}

		void Position::applyMove(std::string_view sv) {
			PieceType pt = charToPieceType[sv[0]];
			assert(pt);
			Piece pc = piece::make(stm, pt);
			Square from;
			Square to;

			Square newEpSquare = NO_SQUARE;
			++rule50Cnt;

			if (pt == PAWN) {
				rule50Cnt = 0;

				// (en passant) capture
				if (sv[1] == 'x' && sv.size() <= 5) {
					to = square::make(file::fromChar(sv[2]), rank::fromChar(sv[3]));
					Direction pawnPush = direction::pawnPush(stm);
					Square capSq = (epSquare && to == epSquare) ? to-pawnPush : to;
					from = square::make(file::fromChar(sv[0]), rank::make(to-pawnPush));
					removePiece(capSq);
					movePiece(from, to);
				}

				// promotion
				else if (sv.size() >= 4 && (sv[2] == '=' || sv[1] == 'x')) {
					Direction pawnPush = direction::pawnPush(stm);

					// capture
					if (sv[1] == 'x') {
						to = square::make(file::fromChar(sv[2]), rank::fromChar(sv[3]));
						removePiece(to);
						from = square::make(file::fromChar(sv[0]), rank::make(to-pawnPush));
						setPiece(to, fenTable[stm ? std::tolower(sv[5]) : sv[5]].piece);
					}
					else {
						to = square::make(file::fromChar(sv[0]), rank::fromChar(sv[1]));
						from = to-pawnPush;
						setPiece(to, fenTable[stm ? std::tolower(sv[3]) : sv[3]].piece);
					}
					removePiece(from);
				}

				// (double) pawn push
				else {
					to = square::make(file::fromChar(sv[0]), rank::fromChar(sv[1]));
					Direction pawnPush = direction::pawnPush(stm);
					from = to-pawnPush;
					if (!piece(from)) {
						newEpSquare = from;
						from -= pawnPush;
					}
					movePiece(from, to);
				}
			}

			else if (pt == KNIGHT) readMove<KNIGHT>(sv);
			else if (pt == BISHOP) readMove<BISHOP>(sv);
			else if (pt == ROOK)   readMove<ROOK>(sv);
			else if (pt == QUEEN)  readMove<QUEEN>(sv);

			else  {
				assert(pt == KING);
				rule50Cnt = 0;

				from = kingSquare(stm);

				// revoke castling rights
				if (stm) castlingRights.data &= ~CastlingRights::BLACK_CASTLING;
				else     castlingRights.data &= ~CastlingRights::WHITE_CASTLING;

				// capture
				if (sv[1] == 'x') {
					to = square::make(file::fromChar(sv[2]), rank::fromChar(sv[3]));

					if (pieceType::make(piece(to)) == ROOK) {
						if (to == square::relative(stm, A8))
							castlingRights.data &= ~(stm ? CastlingRights::BLACK_QUEEN_SIDE : CastlingRights::WHITE_QUEEN_SIDE);

						else if (to == square::relative(stm, H8))
							castlingRights.data &= ~(stm ? CastlingRights::BLACK_KING_SIDE : CastlingRights::WHITE_KING_SIDE);
					}
					removePiece(to);
				}

				// queenside castling
				else if (sv.size() >= 5 && sv.substr(0, 5) == "O-O-O") {
					to = square::relative(stm, C1);
					Square rfrom = square::relative(stm, A1);
					Square rto = square::relative(stm, D1);
					movePiece(rfrom, rto);
				}

				// kingside castling
				else if (sv.size() >= 3 && sv.substr(0, 3) == "O-O") {
					to = square::relative(stm, G1);
					Square rfrom = square::relative(stm, H1);
					Square rto = square::relative(stm, F1);
					movePiece(rfrom, rto);
				}
				
				// quiet move
				else to = square::make(file::fromChar(sv[1]), rank::fromChar(sv[2]));

				movePiece(from, to);
			}

			stm = !stm;
			epSquare = newEpSquare;
			++ply;
		}

		template<PieceType pt> void Position::readMove(std::string_view sv) {

			static_assert(
				pt == KNIGHT ||
				pt == BISHOP ||
				pt == ROOK ||
				pt == QUEEN
			);

			Square from, to;
			bool isCapture;
			uint8_t end;

			if (sv[sv.size()-1] == '+' || sv[sv.size()-1] == '#') {
				to = square::make(file::fromChar(sv[sv.size()-3]), rank::fromChar(sv[sv.size()-2]));

				if (sv[sv.size()-4] == 'x') {
					isCapture = true;
					end = sv.size()-4;
				}
				else {
					isCapture = false;
					end = sv.size()-3;
				}
			}
			else {
				to = square::make(file::fromChar(sv[sv.size()-2]), rank::fromChar(sv[sv.size()-1]));

				if (sv[sv.size()-3] == 'x') {
					isCapture = true;
					end = sv.size()-3;
				}
				else {
					isCapture = false;
					end = sv.size()-2;
				}
			}

			Bitboard targets;
			if (pt == KNIGHT) targets = attacks::knightAttacks[to] & pieces(stm, KNIGHT);
			if (pt == BISHOP) targets = attacks::attacks<BISHOP>(to, occupied) & pieces(stm, BISHOP);
			if (pt == ROOK)   targets = attacks::attacks<ROOK>(to, occupied) & pieces(stm, ROOK);
			if (pt == QUEEN)  targets = attacks::attacks<QUEEN>(to, occupied) & pieces(stm, QUEEN);

			assert(targets);

			if (targets.popcount() == 1)
				from = targets.LSB();

			else {
				bool useFile = false;
				bool useRank = false;
				File file;
				Rank rank;
				for (uint8_t idx = 1; idx < end; ++idx) {
					if (std::isdigit(sv[idx])) {
						useRank = true;
						rank = rank::fromChar(sv[idx]);
					}
					else {
						useFile = true;
						file = file::fromChar(sv[idx]);
					}
				}

				if (useFile)
					targets &= attacks::files[file];

				if (useRank)
					targets &= attacks::ranks[rank];

				Bitboard pinned = this->pinned();
				for (;;) {
					assert(targets);
					from = targets.popLSB();
					if (!pinned.isSet(from)) break;

					if (pt != KNIGHT && pinned.isSet(from)) {
						Square sq = kingSquare(stm);
						int dx_from = file::make(from) - file::make(sq);
						int dy_from = rank::make(from) - rank::make(sq);
						int dx_to = file::make(to) - file::make(sq);
						int dy_to = rank::make(to) - rank::make(sq);

						// north, south
						if (dx_from == 0 || dx_to == 0) {
							if (dx_from != dx_to)
								continue;
						}
						// east, west
						else if (dy_from == 0 || dy_to == 0) {
							if (dy_from != dy_to)
								continue;
						}
						// northeast, southeast, southwest, northwest
						else if (dx_from * dy_to != dy_from * dx_to)
							continue;
						break;
					}
				}
			}
			if (isCapture) removePiece(to);
			if (isCapture && pieceType::make(piece(to)) == ROOK) {
				if (to == square::relative(stm, A8))
					castlingRights.data &= ~(stm ? CastlingRights::BLACK_QUEEN_SIDE : CastlingRights::WHITE_QUEEN_SIDE);

				else if (to == square::relative(stm, H8))
					castlingRights.data &= ~(stm ? CastlingRights::BLACK_KING_SIDE : CastlingRights::WHITE_KING_SIDE);
			}
			movePiece(from, to);
			
			if (pt == ROOK) {
				if (from == square::relative(stm, A1))
					castlingRights.data &= ~(stm ? CastlingRights::BLACK_QUEEN_SIDE : CastlingRights::WHITE_QUEEN_SIDE);

				else if (from == square::relative(stm, H1))
					castlingRights.data &= ~(stm ? CastlingRights::BLACK_KING_SIDE : CastlingRights::WHITE_KING_SIDE);
			}
		}

		void Position::setPiece(Square s, Piece pc) {
			board[s] = pc;
			byPieceType[pieceType::make(pc)].set(s);
			byColor[color::make(pc)].set(s);
			occupied.set(s);
		}

		void Position::removePiece(Square s) {
			byPieceType[pieceType::make(board[s])].clear(s);
			byColor[color::make(board[s])].clear(s);
			board[s] = NO_PIECE;
			occupied.clear(s);
		}

		void Position::movePiece(Square from, Square to) {
			setPiece(to, board[from]);
			removePiece(from);
		}

		Bitboard Position::pinned() const {
			Color them = !stm;
			Square ksq = kingSquare(stm);
			Bitboard ourTeam = piecesByColor(stm);
			Bitboard theirTeam = piecesByColor(them);

			Bitboard pinned = {};
			Bitboard sliderAttackers = theirTeam & (
				attacks::attacks<BISHOP>(ksq, theirTeam) & (pieces(BISHOP) | pieces(QUEEN)) |
				attacks::attacks<ROOK>(ksq, theirTeam) & (pieces(ROOK) | pieces(QUEEN)));

			while (sliderAttackers) {
				Square s = sliderAttackers.popLSB();
				Bitboard between = attacks::inBetweenSquares[ksq][s];
				Bitboard blockers = between & ourTeam;

				if (blockers.popcount() == 1)
					pinned.set(blockers.LSB());
			}
			return pinned;
		}

		inline std::ostream& operator<<(std::ostream& os, const Position& position) {
			const std::string hor = "+---+---+---+---+---+---+---+---+";
			const std::string ver = "|";

			for (Rank r = RANK_8; r >= RANK_1; --r) {
				os << hor;
				os << '\n';

				for (File f = FILE_A; f <= FILE_H; ++f) {
					Piece pc = position.piece(square::make(f, r));
					char c = piece::PIECE_TO_CHAR[pc];
					std::string str = ver + " " + c + " ";
					os << str;
				}
				os << ver;
				os << '\n';
			}
			os << hor;
			os << '\n';
			return os;
		}

	} // namespace pgn

} // namespace chess