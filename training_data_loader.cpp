#include<cstdint>

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

struct SparseBatch {
    using IndexType = int64_t;

    IndexType size;
    IndexType numActiveWhiteFeatures;
    IndexType numActiveBlackFeatures;
    float* stm;
    float* score;
    IndexType* whiteFeatureIndices;
    IndexType* blackFeatureIndices;
    float* whiteFeatureValues;
    float* blackFeatureValues;

    SparseBatch() {
        size = 2048;
        numActiveWhiteFeatures = size * 10;
        numActiveBlackFeatures = numActiveWhiteFeatures;
        stm = new float[size];
        score = new float[size];
        whiteFeatureIndices = new IndexType[numActiveWhiteFeatures * 2];
        blackFeatureIndices = new IndexType[numActiveBlackFeatures * 2];
        whiteFeatureValues = new float[numActiveWhiteFeatures];
        blackFeatureValues = new float[numActiveBlackFeatures];

        for (size_t i = 0; i < size; ++i) {
            stm[i] = i % 2;
        }

        for (size_t i = 0; i < size; ++i) {
            score[i] = 0;
        }

        for (size_t i = 0; i < numActiveWhiteFeatures ; ++i) {
            whiteFeatureIndices[2*i] = i/10;
            whiteFeatureIndices[2*i+1] = 1000*(i%10);
        }

        for (size_t i = 0; i < numActiveBlackFeatures; ++i) {
            blackFeatureIndices[2*i] = i/10;
            blackFeatureIndices[2*i+1] = 1000*(i%10);
        }

        for (size_t i = 0; i < numActiveWhiteFeatures; ++i) {
            whiteFeatureValues[i] = 1;
        }

        for (size_t i = 0; i < numActiveBlackFeatures; ++i) {
            blackFeatureValues[i] = 1;
        }
    }

    ~SparseBatch() {
        delete[] stm;
        delete[] score;
        delete[] whiteFeatureIndices;
        delete[] blackFeatureIndices;
        delete[] whiteFeatureValues;
        delete[] blackFeatureValues;
    }
};

extern "C" {

    EXPORT SparseBatch* CDECL create_sparse_batch() {
        return new SparseBatch;
    }

    EXPORT void CDECL destroy_sparse_batch(SparseBatch* sparseBatch) {
        delete sparseBatch;
    }

} // extern "C"