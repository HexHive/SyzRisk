static BMI2_TARGET_ATTRIBUTE size_t HUF_readStats_body_bmi2(BYTE* huffWeight, size_t hwSize, U32* rankStats,
                     U32* nbSymbolsPtr, U32* tableLogPtr,
                     const void* src, size_t srcSize,
                     void* workSpace, size_t wkspSize)
{
    return HUF_readStats_body(huffWeight, hwSize, rankStats, nbSymbolsPtr, tableLogPtr, src, srcSize, workSpace, wkspSize, 1);
}

struct HUF_readStats_body_bmi2__HEXSHA { void _2aa14b1ab2c4; };
struct HUF_readStats_body_bmi2__ATTRS { };
