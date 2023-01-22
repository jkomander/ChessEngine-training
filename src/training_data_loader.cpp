#include<algorithm> // std::sort
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

using IndexType = uint64_t;

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

    uint16_t pieceIndices[N_COLORS][N_SQUARES][N_PIECES][N_SQUARES];

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
            assert(idx == PIECE_INPUT_SIZE);
        }
    }

    void fillFeatures(
        IndexType i,
        const TrainingDataEntry& e, 
        Color c, 
        IndexType* featureIndices, 
        float* featureValues, 
        IndexType& numActiveFeatures) 
    {
        const Position& pos = e.pos;

        Square ksq = pos.kingSquare(c);
        Bitboard occupied = pos.occupied & ~Bitboard::fromSqure(ksq);

        IndexType active[MAX_ACTIVE_FEATURES];
        IndexType size = 0;

        while (occupied) {
            Square s = occupied.popLSB();
            active[size++] = pieceIndices[c][ksq][pos.piece(s)][s];
        }

        if (pos.canCastle()) {

            IndexType offset = PIECE_INPUT_SIZE + ksq * MISC_SIZE;
            if (pos.canCastle(CastlingRights::WHITE_QUEEN_SIDE)) active[size++] = offset;
            if (pos.canCastle(CastlingRights::WHITE_KING_SIDE))  active[size++] = offset+1;
            if (pos.canCastle(CastlingRights::BLACK_QUEEN_SIDE)) active[size++] = offset+2;
            if (pos.canCastle(CastlingRights::BLACK_KING_SIDE))  active[size++] = offset+3;
        }

        if (pos.epSquare)
            active[size++] = PIECE_INPUT_SIZE + ksq * MISC_SIZE + CASTLING_SIZE + file::make(pos.epSquare);

        // sort the active feature indices
        std::sort(active, active+size);

        for (IndexType j = 0; j < size; ++j) {
            IndexType idx = 2 * numActiveFeatures;
            featureIndices[idx] = i;
            featureIndices[idx+1] = active[j];
            featureValues[numActiveFeatures++] = 1;
        }
    }

} // namespace FeatureTransformer

struct SparseBatch {
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

    SparseBatch(const std::vector<TrainingDataEntry>& entries) {
        using namespace FeatureTransformer;
        assert(size * MAX_ACTIVE_FEATURES * 2 <= std::numeric_limits<IndexType>::max());

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

        for (IndexType i = 0; i < size; ++i)
            fillEntry(i, entries[i]);
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

    void fillEntry(IndexType i, const TrainingDataEntry& e) {
        stm[i] = (float)e.pos.sideToMove;
        score[i] = (float)e.score;
        gameResult[i] = ((float)e.result+1)/2;
        FeatureTransformer::fillFeatures(i, e, WHITE, whiteFeatureIndices, whiteFeatureValues, numActiveWhiteFeatures);
        FeatureTransformer::fillFeatures(i, e, BLACK, blackFeatureIndices, blackFeatureValues, numActiveBlackFeatures);
    }
};

extern "C" {

    EXPORT void CDECL init() {
        FeatureTransformer::init();
        Position::init();
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