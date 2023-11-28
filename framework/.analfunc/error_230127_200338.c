static size_tHUF_compress1X_usingCTable_internal_bmi2("bmi2") size_t
size_tHUF_compress1X_usingCTable_internal_bmi2(ZSTD_DCtx* dctx,
                                 void* dst, size_t maxDstSize,
                           const void* seqStart, size_t seqSize, int nbSeq,
                           const ZSTD_longOffset_e isLongOffset,
                           const int frame)
{
    return ZSTD_decompressSequencesLong_body(dctx, dst, maxDstSize, seqStart, seqSize, nbSeq, isLongOffset, frame);
}

struct size_tHUF_compress1X_usingCTable_internal_bmi2__HEXSHA { void _2aa14b1ab2c4; };
struct size_tHUF_compress1X_usingCTable_internal_bmi2__ATTRS { };
