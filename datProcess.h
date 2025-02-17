#ifndef DAT_PROCESS_H
#define DAT_PROCESS_H

#include <stdio.h>

#include "vdfProcess.h"

#include "common.h"

typedef struct __attribute((packed)) {
    u16 frameCount;
    u16 frames[0]; // Fixed-point multipliers (0..65535 -> 0..16)
} DatKey;

typedef struct __attribute((packed)) {
    u16 keyCount;
    DatKey firstKey[0];
} DatFileHeader;

void DatPreprocess(u8* datData) {}

u32 DatGetFrameCount(u8* datData) {
    DatFileHeader* fileHeader = (DatFileHeader*)datData;

    u32 frameCount = 0;

    DatKey* currentKey = fileHeader->firstKey;
    for (unsigned i = 0; i < fileHeader->keyCount; i++) {
        if (currentKey->frameCount > frameCount)
            frameCount = currentKey->frameCount;
        currentKey = (DatKey*)((u8*)(currentKey + 1) + (currentKey->frameCount * 2));
    }

    return frameCount;
}

float _DatFixedPointToFloat(s16 fixedPoint) {
    return fixedPoint / 4096.0f;
}

float _DatGetInfluenceAtFrame(DatKey* key, float frameNo) {
    unsigned frameLow = (int)frameNo;
    unsigned frameHigh = frameLow + 1;
    float t = frameNo - frameLow;

    float influenceLow = _DatFixedPointToFloat(
        key->frames[MIN(frameLow, key->frameCount - 1)]
    );
    float influenceHigh = _DatFixedPointToFloat(
        key->frames[MIN(frameHigh, key->frameCount - 1)]
    );

    return influenceLow + t * (influenceHigh - influenceLow);
}

void DatApplyVdf(u8* datData, u8* vdfData, TmdVertex* vertices, float frameNo) {
    DatFileHeader* fileHeader = (DatFileHeader*)datData;

    DatKey* currentKey = fileHeader->firstKey;
    for (unsigned i = 0; i < fileHeader->keyCount; i++) {
        if (frameNo < currentKey->frameCount) {
            float influence = _DatGetInfluenceAtFrame(currentKey, frameNo);
            VdfApply(vdfData, i, influence, vertices);
        }

        currentKey = (DatKey*)((u8*)(currentKey + 1) + (currentKey->frameCount * 2));
    }
}

#endif
