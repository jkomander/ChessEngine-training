#pragma once

#include"bitboard.h"

namespace chess {

	namespace attacks {

		inline Bitboard pawnAttacks[N_COLORS][N_SQUARES];
		inline Bitboard knightAttacks[N_SQUARES];
		inline Bitboard kingAttacks[N_SQUARES];
		inline int knightDirections[8] = { 17,10,-6,-15,-17,-10,6,15 };
		inline int kingDirections[8] = { 8,9,1,-7,-8,-9,-1,7 };
		inline int bishopDirections[4] = { 9,-7,-9,7 };
		inline int rookDirections[4] = { 8,1,-8,-1 };

		inline Bitboard files[8] = { FILE_A_BB, FILE_B_BB, FILE_C_BB, FILE_D_BB,
												FILE_E_BB, FILE_F_BB, FILE_G_BB, FILE_H_BB };
		inline Bitboard ranks[8] = { RANK_1_BB, RANK_2_BB, RANK_3_BB, RANK_4_BB,
												RANK_5_BB, RANK_6_BB, RANK_7_BB, RANK_8_BB };
		inline Bitboard diagonals[15];
		inline Bitboard antiDiagonals[15];
		inline Bitboard bishopAttacks[N_SQUARES];
		inline Bitboard rookAttacks[N_SQUARES];

		enum Directions {
			NORTHEAST,
			NORTH,
			NORTHWEST,
			EAST,
			SOUTHEAST,
			SOUTH,
			SOUTHWEST,
			WEST,
			N_DIRECTIONS
		};
		inline Bitboard rayAttacks[N_DIRECTIONS][N_SQUARES];

		inline Bitboard inBetweenSquares[N_SQUARES][N_SQUARES];

		inline Bitboard diagonalsBySquare[N_SQUARES];
		inline Bitboard antiDiagonalsBySquare[N_SQUARES];
		inline Bitboard filesBySquare[N_SQUARES];
		inline Bitboard ranksBySquare[N_SQUARES];
		inline Bitboard firstRankAttacks[8][64];
		inline Bitboard fillUpAttacks[8][64];
		inline Bitboard aFileAttacks[8][64];

		inline void init() {
			// knight attacks
			for (Square from = A1; from < N_SQUARES; ++from) {
				Bitboard attacks = Bitboard();
				for (const auto& d : knightDirections) {
					Square to = from + d;
					if (square::isValid(to) && square::distance(from, to) <= 2)
						attacks.set(to);
				}
				knightAttacks[from] = attacks;
			}

			// king attacks
			for (Square from = A1; from < N_SQUARES; ++from) {
				Bitboard attacks = Bitboard();
				for (const auto& d : kingDirections) {
					Square to = from + d;
					if (square::isValid(to) && square::distance(from, to) == 1)
						attacks.set(to);
				}
				kingAttacks[from] = attacks;
			}

			// diagonals
			diagonals[0] = Bitboard(0x8040201008040201);
			for (int i = 1; i < 8; ++i) {
				diagonals[i] = diagonals[0] << i;
				for (int j = 0; j < i - 1; ++j)
					diagonals[i] -= ranks[7 - j];
			}
			for (int i = 1; i < 8; ++i) {
				diagonals[i + 7] = diagonals[0] >> i;
				for (int j = 0; j < i - 1; ++j)
					diagonals[i + 7] -= ranks[j];
			}

			// anti-diagonals
			for (int i = 0; i < 15; ++i)
				antiDiagonals[i] = diagonals[i].mirrored();

			// bishop attacks on empty board
			for (Square sq = A1; sq < N_SQUARES; ++sq) {
				Bitboard attacks;
				for (auto& b : diagonals) {
					if (b.isSet(sq)) {
						attacks = b;
						break;
					}
				}
				for (auto& b : antiDiagonals) {
					if (b.isSet(sq)) {
						attacks ^= b;
						break;
					}
				}
				bishopAttacks[sq] = attacks;
			}

			// rook attacks on empty board
			for (Square sq = A1; sq < N_SQUARES; ++sq) {
				Bitboard attacks;
				for (auto& b : files) {
					if (b.isSet(sq)) {
						attacks = b;
						break;
					}
				}
				for (auto& b : ranks) {
					if (b.isSet(sq)) {
						attacks ^= b;
						break;
					}
				}
				rookAttacks[sq] = attacks;
			}

			// ray attacks
			for (Square sq = A1; sq < N_SQUARES; ++sq) {
				File file = file::make(sq);
				Rank rank = rank::make(sq);

				rayAttacks[NORTH][sq] = Bitboard(0x101010101010100) << sq;
				rayAttacks[SOUTH][sq] = Bitboard(0x80808080808080) >> 63 - sq;
				rayAttacks[EAST][sq] = Bitboard(0xfe) << sq;
				if (rank < RANK_8)
					rayAttacks[EAST][sq] -= ranks[rank + 1];
				rayAttacks[WEST][sq] = Bitboard(0x7f00000000000000) >> (sq ^ 7 ^ 56);
				if (rank > RANK_1)
					rayAttacks[WEST][sq] -= ranks[rank - 1];
				rayAttacks[NORTHEAST][sq] = Bitboard(0x8040201008040200) << sq;
				for (int i = 0; i < file - rank - 1; ++i)
					rayAttacks[NORTHEAST][sq] -= ranks[7 - i];
				rayAttacks[NORTHWEST][sq] = (Bitboard(0x102040810204000) >> file::make(sq ^ 7)) << 8 * rank;
				for (int i = 7; i > file; --i)
					rayAttacks[NORTHWEST][sq] -= files[i];
				rayAttacks[SOUTHEAST][sq ^ 56] = rayAttacks[NORTHEAST][sq].mirrored();
				rayAttacks[SOUTHWEST][sq ^ 56] = rayAttacks[NORTHWEST][sq].mirrored();
			}

			// pawn attacks
			for (Square sq = A1; sq < N_SQUARES; ++sq) {
				Bitboard b = Bitboard::fromSquare(sq);
				pawnAttacks[WHITE][sq] |= b.shift<7>() | b.shift<9>();
				pawnAttacks[BLACK][sq] |= b.shift<-9>() | b.shift<-7>();
			}

			// squares in between two squares
			for (Square i = A1; i < N_SQUARES; ++i) {
				for (Square j = A1; j < N_SQUARES; ++j) {
					Bitboard b = Bitboard::fromSquare(j);
					for (const auto& arr : rayAttacks) {
						if (arr[i] & b) {
							inBetweenSquares[i][j] = arr[i] - arr[j] - b;
							break;
						}
					}
				}
			}

			for (Square i = 0; i < 64; ++i) {
				Bitboard b = Bitboard::fromSquare(i);
				// diagonals by square
				for (auto& d : diagonals) {
					if (d & b) {
						diagonalsBySquare[i] = d - b;
						break;
					}
				}
				// anti-diagonals by square
				for (auto& d : antiDiagonals) {
					if (d & b) {
						antiDiagonalsBySquare[i] = d - b;
						break;
					}
				}
				// files by square
				for (auto& d : files) {
					if (d & b) {
						filesBySquare[i] = d - b;
						break;
					}
				}
				// ranks by square
				for (auto& d : ranks) {
					if (d & b) {
						ranksBySquare[i] = d - b;
						break;
					}
				}
			}

			for (Square i = 0; i < 8; ++i) {
				for (int j = 0; j < 64; ++j) {

					// first rank attacks,
					Bitboard occ = Bitboard(j<<1) - Bitboard::fromSquare(i);

					for (int k = i-1;; --k) {
						if (k < 0) break;
						firstRankAttacks[i][j].set(k);
						if (occ.isSet(k)) break;
					}
					for (int k = i+1;; ++k) {
						if (k > 7) break;
						firstRankAttacks[i][j].set(k);
						if (occ.isSet(k)) break;
					}

					// fill-up attacks
					fillUpAttacks[i][j] = FILE_A_BB * firstRankAttacks[i][j];

					// A-file attacks
					Square r = i^7;
					occ = Bitboard(j<<1) - Bitboard::fromSquare(r);
					for (int k = r-1;; --k) {
						if (k < 0) break;
						aFileAttacks[i][j].set(8*(k^7));
						if (occ.isSet(k)) break;
					}
					for (int k = r+1;; ++k) {
						if (k > 7) break;
						aFileAttacks[i][j].set(8*(k^7));
						if (occ.isSet(k)) break;
					}
				}
			}
		}

		inline Bitboard diagonalAttacks(Square sq, Bitboard occ) {
			occ = diagonalsBySquare[sq] & occ;
			occ = occ * FILE_B_BB >> 58;
			return fillUpAttacks[file::make(sq)][occ.data] & diagonalsBySquare[sq];
		}

		inline Bitboard antiDiagonalAttacks(Square sq, Bitboard occ) {
			occ = antiDiagonalsBySquare[sq] & occ;
			occ = occ * FILE_B_BB >> 58;
			return fillUpAttacks[file::make(sq)][occ.data] & antiDiagonalsBySquare[sq];
		}

		inline Bitboard rankAttacks(Square sq, Bitboard occ) {
			occ = ranksBySquare[sq] & occ;
			occ = occ * FILE_B_BB >> 58;
			return fillUpAttacks[file::make(sq)][occ.data] & ranksBySquare[sq];
		}

		inline Bitboard fileAttacks(Square sq, Bitboard occ) {
			occ = FILE_A_BB & (occ >> file::make(sq));
			occ = occ * DIAG_C2_H7 >> 58;
			return aFileAttacks[rank::make(sq)][occ.data] << file::make(sq);
		}

		template<PieceType pt>
		inline Bitboard attacks(Square sq, Bitboard occupied) {
			switch (pt) {
			case BISHOP:
				return diagonalAttacks(sq, occupied) | antiDiagonalAttacks(sq, occupied);
			case ROOK:
				return fileAttacks(sq, occupied) | rankAttacks(sq, occupied);
			case QUEEN:
				return diagonalAttacks(sq, occupied) | antiDiagonalAttacks(sq, occupied) |
					fileAttacks(sq, occupied) | rankAttacks(sq, occupied);
			};
		}

	} // namespace attacks

} // namespace chess