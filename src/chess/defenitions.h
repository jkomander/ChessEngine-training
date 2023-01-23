#include<algorithm> // std::max
#include<cstdlib> // std::abs
#include<cstdint>

namespace chess {

	using Color = bool;
	using PieceType = uint8_t;
	using Piece = uint8_t;
	using Square = uint8_t;
	using File = uint8_t;
	using Rank = uint8_t;

	enum Colors {
		WHITE,
		BLACK,
		N_COLORS
	};

	enum PieceTypes {
		NO_PIECE_TYPE,
		PAWN,
		KNIGHT,
		BISHOP,
		ROOK,
		QUEEN,
		KING,
		N_PIECE_TYPES
	};

	enum Pieces {
		NO_PIECE,
		WHITE_PAWN,
		WHITE_KNIGHT,
		WHITE_BISHOP,
		WHITE_ROOK,
		WHITE_QUEEN,
		WHITE_KING,
		BLACK_PAWN = WHITE_PAWN + 8,
		BLACK_KNIGHT,
		BLACK_BISHOP,
		BLACK_ROOK,
		BLACK_QUEEN,
		BLACK_KING,
		N_PIECES
	};

	enum Squares {
		A1, B1, C1, D1, E1, F1, G1, H1,
		A2, B2, C2, D2, E2, F2, G2, H2,
		A3, B3, C3, D3, E3, F3, G3, H3,
		A4, B4, C4, D4, E4, F4, G4, H4,
		A5, B5, C5, D5, E5, F5, G5, H5,
		A6, B6, C6, D6, E6, F6, G6, H6,
		A7, B7, C7, D7, E7, F7, G7, H7,
		A8, B8, C8, D8, E8, F8, G8, H8,
		NO_SQUARE = 0,
		N_SQUARES = 64
	};

	enum Files {
		FILE_A,
		FILE_B,
		FILE_C,
		FILE_D,
		FILE_E,
		FILE_F,
		FILE_G,
		FILE_H,
		N_FILES
	};

	enum Ranks {
		RANK_1,
		RANK_2,
		RANK_3,
		RANK_4,
		RANK_5,
		RANK_6,
		RANK_7,
		RANK_8,
		N_RANKS
	};

	enum Directions {
		NORTH = 8,
		EAST = 1,
		SOUTH = -NORTH,
		WEST = -EAST,
		NORTHEAST = NORTH + EAST,
		NORTHWEST = NORTH + WEST,
		SOUTHEAST = SOUTH + EAST,
		SOUTHWEST = SOUTH + WEST,
		N_DIRECTIONS = 8
	};

	namespace color {
		constexpr Color make(Piece pc) {
			return pc >> 3;
		}
	}

	namespace pieceType {
		constexpr PieceType make(Piece pc) {
			return pc & 7;
		}
	}

	namespace piece {
		constexpr Piece make(Color c, PieceType pt) {
			return (c << 3) + pt;
		}
	}

	namespace square {
		constexpr Square make(File file, Rank rank) {
			return (rank << 3) + file;
		}
	}

	namespace file {
		constexpr File make(Square sq) {
			return sq & 7;
		}

		char chars[N_FILES] = { 'a','b','c','d','e','f','g','h' };
	}

	namespace rank {
		constexpr Rank make(Square sq) {
			return sq >> 3;
		}

		char chars[N_RANKS] = { '1','2','3','4','5','6','7','8' };
	}

	constexpr uint8_t distance(Square s1, Square s2) {
		return std::max(std::abs(file::make(s1) - file::make(s2)), std::abs(rank::make(s1) - rank::make(s2)));
	}

} // namespace chess