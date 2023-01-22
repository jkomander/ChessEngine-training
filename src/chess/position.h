#include<string>
#include<sstream>

#include"bitboard.h"

namespace chess {

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

	struct Position {
		Piece board[N_SQUARES];
		Square ksq[N_COLORS];
		Bitboard occupied;
		Color sideToMove;
		CastlingRights castlingRights;
		Square epSquare;
		uint8_t rule50Cnt;
		uint16_t ply;

		inline const static std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
		
		struct FenTableEntry {
			bool setPiece;
			Piece piece;
			int8_t skip;
		};

		inline static FenTableEntry fenTable[128];

		Position() = default;
		Position(const std::string& fen);

		static Position startPosition() {
			return Position(startFEN);
		}

		static void init();

		bool canCastle(uint8_t flag) const {
			return castlingRights.data & flag;
		}

		bool canCastle() const {
			return castlingRights.data;
		}

		Piece piece(Square s) const {
			return board[s];
		}

		Square kingSquare(Color c) const {
			return ksq[c];
		}
	};

	// From https://github.com/Luecx/CudAD/blob/main/src/position/fenparsing.h
	void Position::init() {
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
	}

	Position::Position(const std::string& fen) {

		std::memset(this, 0, sizeof(Position));
		uint16_t idx = 0;

		// piece placement
		Square sq = A8;

		for (; idx < fen.size() && fen[idx] != ' '; ++idx) {
			const FenTableEntry& e = fenTable[fen[idx]];
			if (e.setPiece) {
				board[sq] = e.piece;
				if (e.piece == WHITE_KING) ksq[WHITE] = sq;
				else if (e.piece == BLACK_KING) ksq[BLACK] = sq;
				occupied.set(sq);
			}
			sq += e.skip;
		}

		++idx;

		// side to move
		sideToMove = fen[idx] == 'w' ? WHITE : BLACK;

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
			epSquare = square::make(file::chars[idx++], rank::chars[idx++]);
		}

		idx += 2;

		// 50 move rule
		if (fen[idx] != '-') {
			auto len = fen.find_first_of(' ', idx);
			auto numeric = std::stoi(fen.substr(idx, len));
			rule50Cnt = numeric;
			idx = len - 1;
		}

		idx += 2;

		// move counter
		if (fen[idx] != '-') {
			auto len = fen.find_first_of(' ', idx);
			auto numeric = std::stoi(fen.substr(idx, len));
			ply = 2 * (numeric - 1) + sideToMove;
			idx = len - 1;
		}
	}

} // namespace chess