#ifndef TMD_PROCESS_H
#define TMD_PROCESS_H

#include <string.h>
#include <math.h>

#include "timProcess.h"

#include "common.h"

#define TMD_HEADER_ID (0x00000041)

typedef struct __attribute((packed)) {
    u32 id; // should match TMD_HEADER_ID
    u32 processed;
    u32 objectCount;
} TmdFileHeader;

typedef struct __attribute((packed)) {
    u32 verticesOffset; // starts from file header
    u32 vertexCount;

    u32 normalsOffset; // starts from file header
    u32 normalCount;

    u32 primitivesOffset; // starts from file header
    u32 primitiveCount;

    s32 scale;
} TmdObjectHeader;

#define GET_TMD_OBJECT_HEADER(tmdData, objectIndex) \
    ((TmdObjectHeader*)((tmdData) + sizeof(TmdFileHeader)) + (objectIndex))

typedef struct __attribute((packed)) {
    s16 x, y, z;
    u16 _pad16;
} TmdVertex;

typedef struct __attribute((packed)) {
    u16 x, y, z;
    u16 _pad16;
} TmdNormal;

typedef struct {
    float x, y, z;
} WorkNormal;

// TmdNormals use 16-bit fixed-point values, so we need to convert them to floats
WorkNormal TmdNormalToWorkNormal(TmdNormal* tmdNormal) {
    WorkNormal workNormal;

    /*
        bit 15 | 14      12 | 11                                          0 |
        sign   | integral   | decimal                                       |
    */

    for (unsigned i = 0; i < 3; i++) {
        u16 fixedPoint = *((u16*)tmdNormal + i);
        float* floatingPoint = (float*)&workNormal + i;

        int signBit = (fixedPoint >> 15) & 0x1;
        int integralPart = (fixedPoint >> 12) & 0x7;
        int decimalPart = fixedPoint & 0xFFF;

        *floatingPoint = integralPart + (decimalPart / 4096.0f);

        if (signBit)
            *floatingPoint = -*floatingPoint;
    }

    return workNormal;
}

typedef struct __attribute((packed)) {
    u8 olen, ilen;
    u8 flag;
    u8 mode;
} TmdPrimitiveHeader;

#define IS_PRIM_POLYGON(primHeader) (((primHeader)->mode >> 5) & 1)

typedef struct __attribute((packed)) {
    u8 rgb[3]; // RGB color for whole triangle
    u8 _mode; // duplicate of mode

    u16 normalIndex; // index into normal table
    u16 vertexIndexes[3]; // indexes into vertex table
} TmdTriangleFlat;

typedef struct __attribute((packed)) {
    u8 rgb0[3]; // RGB color for vertex 0
    u8 _mode; // duplicate of header mode

    u8 rgb1[3]; // RGB color for vertex 1
    u8 _pad8_0;

    u8 rgb2[3]; // RGB color for vertex 2
    u8 _pad8_1;

    u16 normalIndex; // index into normal table
    u16 vertexIndexes[3]; // indexes into vertex table
} TmdTriangleGradated;

typedef struct __attribute((packed)) {
    u8 uv0[2]; // UV coordinates for vertex 0
    u16 cba; // CLUT number [ CBA clutY * 64 + clutX / 16) ]

    u8 uv1[2]; // UV coordinates for vertex 1
    u16 tsb; // Texture Page + Semitransparency Rate (0..3) << 5 + Colour Mode (0..2) << 7

    u8 uv2[2]; // UV coordinates for vertex 2
    u16 _pad16;

    u16 normalIndex; // index into normal table
    u16 vertexIndexes[3]; // indexes into vertex table
} TmdTriangleTextured;

typedef struct __attribute((packed)) {
    u8 rgb[3]; // RGB color for whole triangle
    u8 _mode; // duplicate of mode

    u16 nI0; // normal index for vertex 0
    u16 vI0; // vertex index for vertex 0

    u16 nI1; // normal index for vertex 1
    u16 vI1; // vertex index for vertex 1

    u16 nI2; // normal index for vertex 2
    u16 vI2; // vertex index for vertex 2
} TmdTriangleGouraud;

typedef struct __attribute((packed)) {
    u8 rgb0[3]; // RGB color for vertex 0
    u8 _mode; // duplicate of header mode

    u8 rgb1[3]; // RGB color for vertex 1
    u8 _pad8_0;

    u8 rgb2[3]; // RGB color for vertex 2
    u8 _pad8_1;

    u16 nI0; // normal index for vertex 0
    u16 vI0; // vertex index for vertex 0

    u16 nI1; // normal index for vertex 1
    u16 vI1; // vertex index for vertex 1

    u16 nI2; // normal index for vertex 2
    u16 vI2; // vertex index for vertex 2
} TmdTriangleGouraudGradated;

typedef struct __attribute((packed)) {
    u8 uv0[2]; // UV coordinates for vertex 0
    u16 cba; // position of CLUT in VRAM (use CBA_GET_CBX and CBA_GET_CBY)

    u8 uv1[2]; // UV coordinates for vertex 1
    u16 tsb; // Texture Page + Semitransparency Rate (0..3) << 5 + Colour Mode (0..2) << 7

    u8 uv2[2]; // UV coordinates for vertex 2
    u16 _pad16;

    u16 nI0; // normal index for vertex 0
    u16 vI0; // vertex index for vertex 0

    u16 nI1; // normal index for vertex 1
    u16 vI1; // vertex index for vertex 1

    u16 nI2; // normal index for vertex 2
    u16 vI2; // vertex index for vertex 2
} TmdTriangleGouraudTextured;

#define CBA_GET_CBX(cba) (((u16)cba >> 10) & 0x3F) // upper 6 bits
#define CBA_GET_CBY(cba) ((u16)cba & 0x1FF) // lower 9 bits

#define TSB_GET_TPAGE(tsb) ((u16)tsb & 0x1F) // bits 0-4
#define TSB_GET_ABR(tsb) (((u16)tsb >> 5) & 0x3) // bits 5-6
#define TSB_GET_TPF(tsb) (((u16)tsb >> 7) & 0x3) // bits 7-8

typedef struct __attribute((packed)) {
    u8 rgb[3]; // RGB color for whole triangle
    u8 _mode; // duplicate of mode

    u16 vertexIndexes[3]; // indexes into vertex table
    u16 _pad16;
} TmdTriangleNonlit;

typedef struct __attribute((packed)) {
    u8 uv0[2]; // UV coordinates for vertex 0
    u16 cba; // CLUT number [ CBA clutY * 64 + clutX / 16) ]

    u8 uv1[2]; // UV coordinates for vertex 1
    u16 tsb; // Texture Page + Semitransparency Rate (0..3) << 5 + Colour Mode (0..2) << 7

    u8 uv2[2]; // UV coordinates for vertex 2
    u16 _pad16_0;

    u8 rgb[3]; // Base color for whole triangle
    u8 _pad8;

    u16 vertexIndexes[3]; // indexes into vertex table
    u16 _pad16_1;
} TmdTriangleNonlitTextured;

typedef struct __attribute((packed)) {
    u8 uv0[2]; // UV coordinates for vertex 0
    u16 cba; // CLUT number [ CBA clutY * 64 + clutX / 16) ]

    u8 uv1[2]; // UV coordinates for vertex 1
    u16 tsb; // Texture Page + Semitransparency Rate (0..3) << 5 + Colour Mode (0..2) << 7

    u8 uv2[2]; // UV coordinates for vertex 2
    u16 _pad16_0;

    u8 rgb[3]; // Base color for whole triangle
    u8 _pad8;

    u16 vertexIndexes[3]; // indexes into vertex table
    u16 _pad16_1;
} TmdTriangleNonlitGouraud;

typedef struct __attribute((packed)) {
    u8 rgb[3]; // RGB color for whole line
    u8 _mode; // duplicate of mode

    u16 vertexIndexes[2]; // indexes into vertex table
} TmdLineFlat;

typedef struct __attribute((packed)) {
    u8 rgb0[3]; // RGB color for start of line
    u8 _mode; // duplicate of mode

    u8 rgb1[3]; // RGB color for end of line
    u8 _pad8;

    u16 vertexIndexes[2]; // indexes into vertex table
} TmdLineGradated;

#define HASH_PRIMITIVE_ATTRIBS(flag, mode) ((u32)(flag) ^ ((((u32)(mode)) << 16) | (((u32)(mode)) >> 16)))

typedef struct __attribute__((packed)) {
    struct __attribute__((packed)) {
        u8 isLine : 1;
        u8 isFlat : 1;
        u8 isNonlit : 1;
        u8 isGradated : 1;
        u8 isGouraud : 1;
        u8 isTextured : 1;
        u8 OK : 1; // Did it get loaded in properly?
        u8 reserved : 1;
    } flags;

    u8 rgb0[3];
    u8 rgb1[3];
    u8 rgb2[3];
    u16 uv0[2];
    u16 uv1[2];
    u16 uv2[2];
    u16 tsb;
    s16 vertices[3][3];
    float normals[3][3];
} WorkPrimitive;

typedef struct __attribute((packed)) {
    u8 rgb[3]; // RGB color for whole line
    u8 _pad8;

    u16 vertexIndexes[2]; // indexes into vertex table
} ProLine;

void TmdPreprocess(u8* tmdData) {
    TmdFileHeader* fileHeader = (TmdFileHeader*)tmdData;
    if (fileHeader->id != TMD_HEADER_ID)
        panic("TMD file header ID is nonmatching");
}

u32 TmdGetObjectCount(u8* tmdData) {
    return ((TmdFileHeader*)tmdData)->objectCount;
}

u32 TmdObjectGetVertexCount(u8* tmdData, u32 objectIndex) {
    TmdObjectHeader* objectHeader = GET_TMD_OBJECT_HEADER(tmdData, objectIndex);

    return objectHeader->vertexCount;
}
TmdVertex* TmdObjectGetVertices(u8* tmdData, u32 objectIndex) {
    TmdObjectHeader* objectHeader = GET_TMD_OBJECT_HEADER(tmdData, objectIndex);

    return (TmdVertex*)(tmdData + sizeof(TmdFileHeader) + objectHeader->verticesOffset);
}

u32 TmdObjectGetNormalCount(u8* tmdData, u32 objectIndex) {
    TmdObjectHeader* objectHeader = GET_TMD_OBJECT_HEADER(tmdData, objectIndex);

    return objectHeader->normalCount;
}
TmdNormal* TmdObjectGetNormals(u8* tmdData, u32 objectIndex) {
    TmdObjectHeader* objectHeader = GET_TMD_OBJECT_HEADER(tmdData, objectIndex);

    return (TmdNormal*)(tmdData + sizeof(TmdFileHeader) + objectHeader->normalsOffset);
}

u32 TmdObjectGetPrimitiveCount(u8* tmdData, u32 objectIndex) {
    TmdObjectHeader* objectHeader = GET_TMD_OBJECT_HEADER(tmdData, objectIndex);

    return objectHeader->primitiveCount;
}

float TmdObjectGetScale(u8* tmdData, u32 objectIndex) {
    TmdObjectHeader* objectHeader = GET_TMD_OBJECT_HEADER(tmdData, objectIndex);

    return powf(2.f, (float)objectHeader->scale);
}

WorkPrimitive* TmdObjectCreateWorkPrimitives(u8* tmdData, u32 objectIndex) {
    TmdObjectHeader* objectHeader = GET_TMD_OBJECT_HEADER(tmdData, objectIndex);

    TmdVertex* vertices = (TmdVertex*)(tmdData + sizeof(TmdFileHeader) + objectHeader->verticesOffset);
    TmdNormal* normals = (TmdNormal*)(tmdData + sizeof(TmdFileHeader) + objectHeader->normalsOffset);

    WorkPrimitive* workPrimitives = (WorkPrimitive*)calloc(objectHeader->primitiveCount, sizeof(WorkPrimitive));

    TmdPrimitiveHeader* primitiveHeader =
        (TmdPrimitiveHeader*)(tmdData + sizeof(TmdFileHeader) + objectHeader->primitivesOffset);
    for (unsigned i = 0; i < objectHeader->primitiveCount; i++) {
        void* currentPrimitiveData = (void*)(primitiveHeader + 1);

        int isPolygon = IS_PRIM_POLYGON(primitiveHeader);

        WorkPrimitive* workPrimitive = workPrimitives + i;
        workPrimitive->flags.isLine = !isPolygon;

        switch (HASH_PRIMITIVE_ATTRIBS(primitiveHeader->flag, primitiveHeader->mode)) {
        case HASH_PRIMITIVE_ATTRIBS(0, 0x20): {
            TmdTriangleFlat* tri = (TmdTriangleFlat*)currentPrimitiveData;

            TmdVertex* v0 = vertices + tri->vertexIndexes[0];
            TmdVertex* v1 = vertices + tri->vertexIndexes[1];
            TmdVertex* v2 = vertices + tri->vertexIndexes[2];

            workPrimitive->flags.isFlat = 1;
            workPrimitive->flags.isNonlit = 0;
            workPrimitive->flags.isGradated = 0;
            workPrimitive->flags.isGouraud = 0;
            workPrimitive->flags.isTextured = 0;

            memcpy(workPrimitive->rgb0, tri->rgb, 3);
            memcpy(workPrimitive->rgb1, tri->rgb, 3);
            memcpy(workPrimitive->rgb2, tri->rgb, 3);

            memcpy(workPrimitive->vertices[0], v0, sizeof(TmdVertex));
            memcpy(workPrimitive->vertices[1], v1, sizeof(TmdVertex));
            memcpy(workPrimitive->vertices[2], v2, sizeof(TmdVertex));

            memset(workPrimitive->normals, 0, sizeof(float) * 3 * 3);

            workPrimitive->flags.OK = 1;
        } break;

        case HASH_PRIMITIVE_ATTRIBS(0, 0x30): {
            TmdTriangleGouraud* tri = (TmdTriangleGouraud*)currentPrimitiveData;

            TmdVertex* v0 = vertices + tri->vI0;
            TmdVertex* v1 = vertices + tri->vI1;
            TmdVertex* v2 = vertices + tri->vI2;

            WorkNormal n0 = TmdNormalToWorkNormal(normals + tri->nI0);
            WorkNormal n1 = TmdNormalToWorkNormal(normals + tri->nI1);
            WorkNormal n2 = TmdNormalToWorkNormal(normals + tri->nI2);

            workPrimitive->flags.isFlat = 0;
            workPrimitive->flags.isNonlit = 0;
            workPrimitive->flags.isGradated = 0;
            workPrimitive->flags.isGouraud = 1;
            workPrimitive->flags.isTextured = 0;

            memcpy(workPrimitive->rgb0, tri->rgb, 3);
            memcpy(workPrimitive->rgb1, tri->rgb, 3);
            memcpy(workPrimitive->rgb2, tri->rgb, 3);

            memcpy(workPrimitive->vertices[0], v0, sizeof(TmdVertex));
            memcpy(workPrimitive->vertices[1], v1, sizeof(TmdVertex));
            memcpy(workPrimitive->vertices[2], v2, sizeof(TmdVertex));

            memcpy(workPrimitive->normals[0], &n0, sizeof(WorkNormal));
            memcpy(workPrimitive->normals[1], &n1, sizeof(WorkNormal));
            memcpy(workPrimitive->normals[2], &n2, sizeof(WorkNormal));

            workPrimitive->flags.OK = 1;
        } break;
        
        case HASH_PRIMITIVE_ATTRIBS(0, 0x40):
        case HASH_PRIMITIVE_ATTRIBS(1, 0x40): {
            TmdLineFlat* line = (TmdLineFlat*)currentPrimitiveData;

            TmdVertex* v0 = vertices + line->vertexIndexes[0];
            TmdVertex* v1 = vertices + line->vertexIndexes[1];

            workPrimitive->flags.isFlat = 1;
            workPrimitive->flags.isNonlit = 0;
            workPrimitive->flags.isGradated = 0;
            workPrimitive->flags.isGouraud = 0;
            workPrimitive->flags.isTextured = 0;

            memcpy(workPrimitive->rgb0, line->rgb, 3);
            memcpy(workPrimitive->rgb1, line->rgb, 3);
            memcpy(workPrimitive->rgb2, line->rgb, 3);

            memcpy(workPrimitive->vertices[0], v0, sizeof(TmdVertex));
            memcpy(workPrimitive->vertices[1], v1, sizeof(TmdVertex));
            memcpy(workPrimitive->vertices[2], v1, sizeof(TmdVertex));

            memset(workPrimitive->normals, 0, sizeof(float) * 3 * 3);

            workPrimitive->flags.OK = 1;
        } break;

        case HASH_PRIMITIVE_ATTRIBS(1, 0x21): {
            TmdTriangleNonlit* tri = (TmdTriangleNonlit*)currentPrimitiveData;

            TmdVertex* v0 = vertices + tri->vertexIndexes[0];
            TmdVertex* v1 = vertices + tri->vertexIndexes[1];
            TmdVertex* v2 = vertices + tri->vertexIndexes[2];

            workPrimitive->flags.isFlat = 0;
            workPrimitive->flags.isNonlit = 1;
            workPrimitive->flags.isGradated = 0;
            workPrimitive->flags.isGouraud = 0;
            workPrimitive->flags.isTextured = 0;

            memcpy(workPrimitive->rgb0, tri->rgb, 3);
            memcpy(workPrimitive->rgb1, tri->rgb, 3);
            memcpy(workPrimitive->rgb2, tri->rgb, 3);

            memcpy(workPrimitive->vertices[0], v0, sizeof(TmdVertex));
            memcpy(workPrimitive->vertices[1], v1, sizeof(TmdVertex));
            memcpy(workPrimitive->vertices[2], v2, sizeof(TmdVertex));

            memset(workPrimitive->normals, 0, sizeof(float) * 3 * 3);

            workPrimitive->flags.OK = 1;
        } break;

        case HASH_PRIMITIVE_ATTRIBS(1, 0x25): {
            TmdTriangleNonlitTextured* tri = (TmdTriangleNonlitTextured*)currentPrimitiveData;

            TmdVertex* v0 = vertices + tri->vertexIndexes[0];
            TmdVertex* v1 = vertices + tri->vertexIndexes[1];
            TmdVertex* v2 = vertices + tri->vertexIndexes[2];

            //u32 cbx = CBA_GET_CBX(tri->cba);
            //u32 cby = CBA_GET_CBY(tri->cba);

            u32 tpage = TSB_GET_TPAGE(tri->tsb);

            u16 pageX = (tpage * VR_PAGE_WIDTH32) % VR_WIDTH32;
            u16 pageY = tpage >= 16 ? VR_PAGE_HEIGHT : 0;

            workPrimitive->flags.isFlat = 0;
            workPrimitive->flags.isNonlit = 1;
            workPrimitive->flags.isGradated = 0;
            workPrimitive->flags.isGouraud = 0;
            workPrimitive->flags.isTextured = 1;

            memcpy(workPrimitive->rgb0, tri->rgb, 3);
            memcpy(workPrimitive->rgb1, tri->rgb, 3);
            memcpy(workPrimitive->rgb2, tri->rgb, 3);

            memcpy(workPrimitive->vertices[0], v0, sizeof(TmdVertex));
            memcpy(workPrimitive->vertices[1], v1, sizeof(TmdVertex));
            memcpy(workPrimitive->vertices[2], v2, sizeof(TmdVertex));

            memset(workPrimitive->normals, 0, sizeof(float) * 3 * 3);

            workPrimitive->uv0[0] = pageX + tri->uv0[0];
            workPrimitive->uv0[1] = pageY + tri->uv0[1];
            workPrimitive->uv1[0] = pageX + tri->uv1[0];
            workPrimitive->uv1[1] = pageY + tri->uv1[1];
            workPrimitive->uv2[0] = pageX + tri->uv2[0];
            workPrimitive->uv2[1] = pageY + tri->uv2[1];

            // TODO: figure this out
            workPrimitive->tsb = tri->tsb;

            workPrimitive->flags.OK = 1;
        } break;

        default:
            workPrimitive->flags.OK = 0;
            break;
        }

        primitiveHeader = (TmdPrimitiveHeader*)(
            (u8*)(primitiveHeader + 1) + (primitiveHeader->ilen * 4)
        );
    }

    return workPrimitives;
}

#endif
