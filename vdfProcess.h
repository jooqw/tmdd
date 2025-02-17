#ifndef VDF_PROCESS_H
#define VDF_PROCESS_H

#include "tmdProcess.h"

#include <stdio.h>

#include "common.h"

typedef struct __attribute((packed)) {
    s16 x, y, z;
    u16 _pad16;
} VdfVertex;

typedef struct __attribute((packed)) {
    u32 objectIndex;
    u32 firstVertex;
    u32 vertexCount;

    VdfVertex vertices[0];
} VdfKey;

typedef struct __attribute((packed)) {
    u32 keyCount;

    VdfKey firstKey[0];
} VdfFileHeader;

void VdfPreprocess(u8* vdfData) {}

u32 VdfGetKeyCount(u8* vdfData) {
    return ((VdfFileHeader*)vdfData)->keyCount;
}

VdfKey* _VdfGetKeyFromIndex(u8* vdfData, u32 keyIndex) {
    VdfKey* key = ((VdfFileHeader*)vdfData)->firstKey;
    for (unsigned i = 0; i < keyIndex; i++)
        key = (VdfKey*)((u8*)(key + 1) + key->vertexCount * sizeof(VdfVertex));

    return key;
}

u32 VdfGetKeyObjectIndex(u8* vdfData, u32 keyIndex) {
    return (_VdfGetKeyFromIndex(vdfData, keyIndex))->objectIndex;
}

void VdfApply(u8* vdfData, u32 keyIndex, float influence, TmdVertex* vertices) {
    VdfKey* key = _VdfGetKeyFromIndex(vdfData, keyIndex);

    TmdVertex* vertex = (TmdVertex*)((u8*)vertices + key->firstVertex);
    for (unsigned i = 0; i < key->vertexCount; i++, vertex++) {
        VdfVertex* vdfVertex = key->vertices + i;

        vertex->x += (vdfVertex->x * influence);
        vertex->y += (vdfVertex->y * influence);
        vertex->z += (vdfVertex->z * influence);
    }
}

#endif
