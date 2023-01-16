#include<cassert>
#include<numeric>
#include<iostream>
#include<vector>

#include"chess/position.h"

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

using namespace chess;

struct TrainingDataEntry {
    Position pos;
    float score;
    float result;
};

namespace FeatureTransformer {

    template<class T>
    constexpr T ceilToMultiple(T n, T r) {
        return (n + r - 1) / r * r;
    }

    constexpr int MAX_ACTIVE_FEATURES = 37;

    constexpr int CASTLING_SIZE = 4;
    constexpr int EN_PASSANT_SIZE = 8;
    constexpr int MISC_SIZE = CASTLING_SIZE + EN_PASSANT_SIZE;
    constexpr int PIECE_INPUT_SIZE = 41916;
    constexpr int MISC_INPUT_SIZE = N_SQUARES * MISC_SIZE;  

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

                            Piece pc = piece::make(c_, pt);

                            if (pc == piece::make(c, KING) ||
                                psq == ksq ||
                                pc == piece::make(!c, KING) && distance(psq, ksq) == 1 ||
                                pt == PAWN && (RANK_1_BB | RANK_8_BB).isSet(psq))
                                continue;

                            pieceIndices[c][ksq][pc][psq] = idx++;
                        }
                    }
                }
            }
            std::cout << idx << " " << PIECE_INPUT_SIZE << "\n";
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

    SparseBatch(std::vector<TrainingDataEntry>& entries) {
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
            TrainingDataEntry& entry = entries[i];

            stm[i] = (float)entry.pos.sideToMove;
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
        std::vector<TrainingDataEntry>entries{};
        for (size_t i = 0; i < batch_size; ++i) {
            entries.emplace_back();
            entries[i].pos = Position::startPosition();
            entries[i].score = -42;
            entries[i].result = float(i % 3)/2;
        }
        return new SparseBatch(entries);
    }

    EXPORT void CDECL destroy_sparse_batch(SparseBatch* sparseBatch) {
        delete sparseBatch;
    }

} // extern "C"