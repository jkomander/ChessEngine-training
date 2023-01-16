#include<string>

#include"bitboard.h"

namespace chess {

	struct CastlingRights {

	};

	struct Position {
		Piece board[N_PIECES];
		Color sideToMove;
		Square epSquare;
		CastlingRights castlingRights;
		uint8_t rule50Cnt;
		uint16_t ply;

		inline const static std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

		Position() = default;
		Position(std::string_view fen) { set(fen); }

		static Position startPosition() {
			return Position(startFEN);
		}

		void set(std::string_view fen) {

		}
	};

} // namespace chess