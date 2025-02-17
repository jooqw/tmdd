#ifndef TIM_PROCESS_H
#define TIM_PROCESS_H

#include <stdio.h>

#include <string.h>

#include "common.h"

#define VR_PAGE_WIDTH (64)
#define VR_PAGE_HEIGHT (256)

#define VR_WIDTH (VR_PAGE_WIDTH * 16)
#define VR_HEIGHT (VR_PAGE_HEIGHT * 2)

#define VR_PAGE_WIDTH32 (VR_PAGE_WIDTH * 4)
#define VR_WIDTH32 (VR_PAGE_WIDTH32 * 16)

#define TIM_HEADER_ID      0x10
#define TIM_HEADER_VERSION 0x00

typedef struct __attribute((packed)) {
    u8 id; // should match TIM_HEADER_ID
    u8 version; // should match TIM_HEADER_VERSION
    u16 _reserved;

    u32 flag;
} TimFileHeader;

#define TIM_HEADER_FLAG_PMODE(flag) ((u32)flag & 0x07)
#define TIM_PMODE_4BIT_CLUT 0
#define TIM_PMODE_8BIT_CLUT 1
#define TIM_PMODE_15BIT_DIRECT 2
#define TIM_PMODE_24BIT_DIRECT 3
#define TIM_PMODE_MIXED 4

#define TIM_HEADER_FLAG_CF(flag)    ((u32)flag & 0x10)

typedef struct __attribute((packed)) {
    u32 clutSectionSize; // includes header
    u16 fbX, fbY;
    u16 width, height;

    u16 entries[0];
} TimCLUTHeader;

#define TIM_CLUT_ENTRY_STP(entry) (((u16)entry >> 15) & 0x01)
#define TIM_CLUT_ENTRY_R(entry)   (((u16)entry >>  0) & 0x1F)
#define TIM_CLUT_ENTRY_G(entry)   (((u16)entry >>  5) & 0x1F)
#define TIM_CLUT_ENTRY_B(entry)   (((u16)entry >> 10) & 0x1F)

typedef struct __attribute((packed)) {
    u32 pixelSectionSize; // includes header
    u16 fbX, fbY;
    u16 width; // in 16-bit units
    u16 height;

    u8 data[0];
} TimPixelHeader;

void TimPreprocess(u8* timData) {
    TimFileHeader* fileHeader = (TimFileHeader*)timData;
    if (fileHeader->id != TIM_HEADER_ID)
        panic("TIM file header ID is nonmatching");
    if (fileHeader->version != TIM_HEADER_VERSION)
        panic("TIM file header version is nonmatching");
}

void _TimDecodePixels(TimFileHeader* fileHeader, u32 paletteIndex, u32* pixels) {
    TimCLUTHeader* clutHeader = (TimCLUTHeader*)(fileHeader + 1);
    TimPixelHeader* pixelHeader = (TimPixelHeader*)((u8*)clutHeader + clutHeader->clutSectionSize);

    u32 numPixels = pixelHeader->width * pixelHeader->height * 4;

    u32 paletteOffset = paletteIndex * clutHeader->width;
    for (unsigned i = 0; i < numPixels; ++i) {
        u8 colorIndex = pixelHeader->data[i / 2];
        if (i % 2)
            colorIndex = ((colorIndex & 0xF0) >> 4);
        else
            colorIndex = colorIndex & 0x0F;

        u16 entry = clutHeader->entries[paletteOffset + colorIndex];

        u8 raw_r = TIM_CLUT_ENTRY_R(entry);
        u8 raw_g = TIM_CLUT_ENTRY_G(entry);
        u8 raw_b = TIM_CLUT_ENTRY_B(entry);
        u8 r = (u8)roundf(raw_r * 255.f / 31.f);
        u8 g = (u8)roundf(raw_g * 255.f / 31.f);
        u8 b = (u8)roundf(raw_b * 255.f / 31.f);

        // u8 stp = TIM_CLUT_ENTRY_STP(entry);

        u8 a = 0xFFu;
        if (entry == 0x0000)
            a = 0x00u;

        pixels[i] = (a << 24) | (b << 16) | (g << 8) | r;
    }
}

void TimVrCopy(u8* timData, u8* vr) {
    TimFileHeader* fileHeader   = (TimFileHeader*)timData;
    TimCLUTHeader* clutHeader   = (TimCLUTHeader*)(fileHeader + 1);
    TimPixelHeader* pixelHeader = (TimPixelHeader*)((u8*)clutHeader + clutHeader->clutSectionSize);

    u32 width = pixelHeader->width * 4;
    u32 height = pixelHeader->height;

    u8* imageData = malloc(width * height * 4);
    _TimDecodePixels(fileHeader, 0, (u32*)imageData);

    for (unsigned row = 0; row < height; row++) {
        u8* src = imageData + (row * width * 4);
        u8* dst = vr + ((row + pixelHeader->fbY) * VR_WIDTH32 + (pixelHeader->fbX * 4)) * 4;

        for (unsigned col = 0; col < width; col++) {
            u8* srcPixel = src + (col * 4);
            u8* dstPixel = dst + (col * 4);

            u8 alpha = srcPixel[3];

            // Overwrite the pixel
            if (alpha == 255) {
                *(u32*)dstPixel = *(u32*)srcPixel;
            }
            // Blend the pixel
            else if (alpha > 0) {
                dstPixel[0] = (srcPixel[0] * alpha + dstPixel[0] * (255 - alpha)) / 255; // R
                dstPixel[1] = (srcPixel[1] * alpha + dstPixel[1] * (255 - alpha)) / 255; // G
                dstPixel[2] = (srcPixel[2] * alpha + dstPixel[2] * (255 - alpha)) / 255; // B
                dstPixel[3] = dstPixel[3] > alpha ? dstPixel[3] : alpha;
            }
        }
    }

    free(imageData);
}

#endif
