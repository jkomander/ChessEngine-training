#include<cstdint>

#include"lib/nnue_training_data_formats.h"
#include"lib/nnue_training_data_stream.h"
#include"lib/rng.h"

#if defined (__x86_64__)
#define EXPORT
#define CDECL
#else
#if defined (_MSC_VER)
#define EXPORT __declspec(dllexport)
#define CDECL __cdecl
#else
#define EXPORT
#define CDECL __attribute__ ((__cdecl__))
#endif
#endif

namespace FeatureTransformer {

    namespace ChessEngine {

		using Color = bool;
		using Piece = uint8_t;
		using PieceType = uint8_t;
		using Square = uint8_t;
        using File = uint8_t;
        using Rank = uint8_t;

		enum Colors : Color {
			WHITE,
			BLACK,
			N_COLORS
		};

		enum Pieces : Piece{
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

		enum PieceTypes : PieceType {
			NO_PIECE_TYPE,
			PAWN,
			KNIGHT,
			BISHOP,
			ROOK,
			QUEEN,
			KING,
			N_PIECE_TYPES
		};

		enum Squares : Square{
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

        enum Files : File {
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

        enum Ranks : Rank {
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

		namespace sq {
			constexpr Piece piece(Color c, PieceType pt) {
				assert(pt != NO_PIECE_TYPE);
				return c * 8 + pt;
			}

			constexpr Color color(Piece pc) {
				return pc & 8;
			}

			constexpr PieceType pieceType(Piece pc) {
				return pc & 7;
			}

            constexpr File file(Square sq) { return sq % 8; }
            constexpr Rank rank(Square sq) { return sq / 8; }

            int distance(Square s1, Square s2) {
                return std::max(std::abs(file(s1) - file(s2)), std::abs(rank(s1) - rank(s2)));
            }

		}

        struct Bitboard {
            uint64_t data;

            Bitboard() = default;
            constexpr Bitboard(uint64_t data) :data(data) {}
            static Bitboard fromSquare(Square sq) { return { uint64_t(1) << sq }; }

            bool at(Square sq) { return data & uint64_t(1) << sq; }
            void set(Square sq) { data |= uint64_t(1) << sq; }
            void clear(Square sq) { data &= ~(uint64_t(1) << sq); }

            friend Bitboard operator&(const Bitboard& a, const Bitboard& b) {
                return { a.data & b.data };
            }

            friend Bitboard operator|(const Bitboard& a, const Bitboard& b) {
                return { a.data | b.data };
            }

            friend Bitboard operator^(const Bitboard& a, const Bitboard& b) {
                return { a.data ^ b.data };
            }

            friend Bitboard operator-(const Bitboard& a, const Bitboard& b) {
                return { a.data & ~b.data };
            }

            friend Bitboard operator*(const Bitboard& a, const Bitboard& b) {
                return { a.data * b.data };
            }

            void operator&=(const Bitboard& b) {
                this->data &= b.data;
            }

            void operator|=(const Bitboard& b) {
                this->data |= b.data;
            }

            void operator^=(const Bitboard& b) {
                this->data ^= b.data;
            }

            void operator-=(const Bitboard& b) {
                this->data &= ~b.data;
            }

            Bitboard operator~() {
                return { ~this->data };
            }

            explicit operator bool() {
                return bool(this->data);
            }

            constexpr friend Bitboard operator<<(const Bitboard& b, int n) {
                return{ b.data << n };
            }

            friend Bitboard operator>>(const Bitboard& b, int n) {
                return{ b.data >> n };
            }

            bool operator==(const Bitboard& b) {
                return data == b.data;
            }

        };

        constexpr Bitboard RANK_1_BB = Bitboard(0xff);
        constexpr Bitboard RANK_2_BB = RANK_1_BB << 8;
        constexpr Bitboard RANK_3_BB = RANK_1_BB <<	16;
        constexpr Bitboard RANK_4_BB = RANK_1_BB << 24;
        constexpr Bitboard RANK_5_BB = RANK_1_BB << 32;
        constexpr Bitboard RANK_6_BB = RANK_1_BB << 40;
        constexpr Bitboard RANK_7_BB = RANK_1_BB << 48;
        constexpr Bitboard RANK_8_BB = RANK_1_BB << 56;

	} // namespace ChessEngine

    using namespace ChessEngine;

    template<class T>
    constexpr T ceilToMultiple(T n, T r) {
        return (n + r - 1) / r * r;
    }

    constexpr int MAX_ACTIVE_FEATURES = 37;

    constexpr int CASTLING_SIZE = 4;
    constexpr int EN_PASSANT_SIZE = 8;
    constexpr int MISC_SIZE = CASTLING_SIZE + EN_PASSANT_SIZE;
    constexpr int PIECE_INPUT_SIZE = 41916;
    constexpr int MISC_INPUT_SIZE = ChessEngine::N_SQUARES * MISC_SIZE;  

    constexpr int NUM_FEATURES = PIECE_INPUT_SIZE + MISC_INPUT_SIZE;
    constexpr int PADDED_NUM_FEATURES = ceilToMultiple(NUM_FEATURES, 16);
    constexpr int INPUT_HSIZE = PADDED_NUM_FEATURES;
    constexpr int INPUT_SIZE = 2*INPUT_HSIZE;
    constexpr int ACCUMULATOR_SIZE = 256;
    constexpr int ACCUMULATOR_DSIZE = 2*ACCUMULATOR_SIZE;
    constexpr int HIDDEN_1_SIZE = 32;
    constexpr int HIDDEN_2_SIZE = 32;

    using IndexType = uint16_t;
    IndexType pieceIndices[N_COLORS][N_SQUARES][N_PIECES][N_SQUARES];

    void init() {
        // initialize the index lookup table
        size_t idx;
        for (Color c : { WHITE, BLACK }) {
            idx = 0;
            for (Square ksq = A1; ksq < N_SQUARES; ++ksq) {
                for (Color c_ : { WHITE, BLACK }) {
                    for (PieceType pt = PAWN; pt < N_PIECE_TYPES; ++pt) {
                        for (Square psq = A1; psq < N_SQUARES; ++psq) {

                            Piece pc = sq::piece(c_, pt);

                            if (pc == sq::piece(c, KING) ||
                                psq == ksq ||
                                pc == sq::piece(!c, KING) && sq::distance(psq, ksq) == 1 ||
                                pt == PAWN && (RANK_1_BB | RANK_8_BB).at(psq))
                                continue;

                            pieceIndices[c][ksq][pc][psq] = idx++;
                        }
                    }
                }
            }
            assert(idx == PIECE_INPUT_SIZE);
        }
    }

} // namespace FeatureTransformer

struct SparseBatch {
    using IndexType = size_t;

    IndexType size;
    IndexType numActiveWhiteFeatures;
    IndexType numActiveBlackFeatures;
    float* stm;
    float* score;
    float* gameResult;
    IndexType* whiteFeatureIndices;
    IndexType* blackFeatureIndices;
    float* whiteFeatureValues;
    float* blackFeatureValues;

    SparseBatch() = default;

    SparseBatch(std::vector<binpack::TrainingDataEntry>& entries) {
        using namespace FeatureTransformer;
        assert(size * MAX_ACTIVE_FEATURES * 2 <= std::numeric_limits<int64_t>::max());

        size = entries.size();
        numActiveWhiteFeatures = 0;
        numActiveBlackFeatures = 0;
        stm = new float[size];
        score = new float[size];
        gameResult = new float[size];
        whiteFeatureIndices = new IndexType[size * MAX_ACTIVE_FEATURES * 2];
        blackFeatureIndices = new IndexType[size * MAX_ACTIVE_FEATURES * 2];
        whiteFeatureValues = new float[size * MAX_ACTIVE_FEATURES];
        blackFeatureValues = new float[size * MAX_ACTIVE_FEATURES];

        for (size_t i = 0; i < size; ++i) {
            binpack::TrainingDataEntry& entry = entries[i];

            stm[i] = (float)entry.pos.sideToMove();
            score[i] = (float)entry.score;
            gameResult[i] = ((float)entry.result+1)/2;

            numActiveWhiteFeatures = 10*size;
            numActiveBlackFeatures = numActiveWhiteFeatures;

            for (size_t j = 0; j < 10; ++j) {
                whiteFeatureIndices[2*(10*i+j)] = i;
                blackFeatureIndices[2*(10*i+j)] = i;
                whiteFeatureIndices[2*(10*i+j)+1] = j*1000;
                blackFeatureIndices[2*(10*i+j)+1] = j*1000;
                whiteFeatureValues[10*i+j] = 1;
                blackFeatureValues[10*i+j] = 1;
            }
        }
    }

    ~SparseBatch() {
        delete[] stm;
        delete[] score;
        delete[] gameResult;
        delete[] whiteFeatureIndices;
        delete[] blackFeatureIndices;
        delete[] whiteFeatureValues;
        delete[] blackFeatureValues;
    }
};

extern "C" {

    EXPORT void CDECL init() {
        FeatureTransformer::init();
    }

    EXPORT SparseBatch* CDECL create_sparse_batch(size_t batch_size) {
        std::vector<binpack::TrainingDataEntry>entries{};
        for (size_t i = 0; i < batch_size; ++i) {
            entries.emplace_back();
            entries[i].pos = chess::Position::startPosition();
            entries[i].score = -42;
            entries[i].result = float(i % 3)/2;
        }
        return new SparseBatch(entries);
    }

    EXPORT void CDECL destroy_sparse_batch(SparseBatch* sparseBatch) {
        delete sparseBatch;
    }

} // extern "C"