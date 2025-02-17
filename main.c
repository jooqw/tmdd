#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include <raylib.h>
#include <raymath.h>

#include "tmdProcess.h"
#include "timProcess.h"
#include "vdfProcess.h"
#include "datProcess.h"

#include "model.h"

#include "common.h"

#define TARGET_FPS (60)
#define WINDOW_WIDTH (800)
#define WINDOW_HEIGHT (600)

void ReadBinary(char* path, u8** bufferOut, u64* sizeOut) {
    FILE* fpBin = fopen(path, "rb");
    if (fpBin == NULL)
        panic("The binary could not be opened.");

    fseek(fpBin, 0, SEEK_END);
    u64 bufSize = ftell(fpBin);
    rewind(fpBin);

    u8* buffer = (u8 *)malloc(bufSize);
    if (buffer == NULL) {
        fclose(fpBin);

        panic("Failed to allocate bin buf");
    }

    u64 bytesCopied = fread(buffer, 1, bufSize, fpBin);
    if (bytesCopied != bufSize) {
        free(buffer);
        fclose(fpBin);

        panic("Buffer readin fail");
    }

    fclose(fpBin);

    *bufferOut = buffer;
    if (sizeOut)
        *sizeOut = bufSize;
}

typedef struct {
    char* tmdFile;

    unsigned timCount;
    char** timFiles;
    char* vdfFile;

    char* datFile;
} Arguments;

void usage() {
    printf(
        "Usage: tmdd -t <TMD file> [-i <TIM files>...] [-v <VDF file>] [-d <DAT file>]\n"
        "  -t <TMD file>      : Path to the TMD geometry file.\n"
        "  -i <TIM files>...  : All associated TIM texture files. If none are passed,\n"
        "                       the model will be displayed in wireframe mode.\n"
        "  -v <VDF file>      : Path to a VDF mime file (optional).\n"
        "  -d <DAT file>      : Path to a DAT animation file (optional).\n"
        "                       This file is exclusively present in Parappa the Rapper &\n"
        "                       Um Jammer Lammy.\n"
    );
}

Arguments parseArguments(int argc, char** argv) {
    Arguments args = { 0 };
    args.timFiles = malloc(argc * sizeof(char*));

    int opt;
    while ((opt = getopt(argc, argv, "t:i:v:d:")) != -1) {
        switch (opt) {
            case 't': {
                args.tmdFile = optarg;
            } break;
            case 'i': {
                args.timFiles[args.timCount++] = optarg;
                while (optind < argc && argv[optind][0] != '-')
                    args.timFiles[args.timCount++] = argv[optind++];
            } break;
            case 'v': {
                args.vdfFile = optarg;
            } break;
            case 'd': {
                args.datFile = optarg;
            } break;

            default: {
                usage();
                exit(1);
            }
        }
    }

    if (!args.tmdFile) {
        fprintf(stderr, "Error: TMD file is required.\n");
        usage();
        exit(1);
    }

    return args;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        usage();
        return 1;
    }

    Arguments args = parseArguments(argc, argv);

    const int noTexture = args.timCount == 0;
    const int onlyVdf = !!args.vdfFile && !args.datFile;
    const int canAnimate = !!args.vdfFile && !onlyVdf;

    u8* tmdData;
    u64 tmdDataSize;

    u8* vdfData = NULL;
    u8* datData = NULL;

    printf("Read & copy TMD binary ..");

    ReadBinary(args.tmdFile, &tmdData, &tmdDataSize);

    TmdPreprocess(tmdData);

    LOG_OK;

    Image iMat = { 0 };
    if (!noTexture) {
        printf("Load & process TIM binaries ..");

        iMat.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        iMat.width = VR_WIDTH32;
        iMat.height = VR_HEIGHT;
        iMat.data = calloc(iMat.width * iMat.height, 4);
        iMat.mipmaps = 1;

        for (unsigned i = 0; i < args.timCount; i++) {
            u8* timData;
            u64 timDataSize;
            ReadBinary(args.timFiles[i], &timData, &timDataSize);

            TimPreprocess(timData);

            TimVrCopy(timData, (u8*)iMat.data);

            free(timData);
        }

        LOG_OK;
    }

    if (args.vdfFile) {
        printf("Load & process VDF ..");

        ReadBinary(args.vdfFile, &vdfData, NULL);
        VdfPreprocess(vdfData);

        LOG_OK;
    }

    if (args.datFile) {
        printf("Load & process DAT ..");

        ReadBinary(args.datFile, &datData, NULL);
        DatPreprocess(datData);

        LOG_OK;
    }

    // Init scene

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "tmdd");

    Camera camera = { 0 };
    camera.position = (Vector3){ 0.f, 20.f, 50.f }; // Camera position
    camera.target = (Vector3){ 0.0f, 10.0f, 0.0f };     // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    ModelData* model = ModelCreate(tmdData, tmdDataSize);
    if (noTexture) {
        ModelApplyDefaultMaterial(model);
        model->tint = BLACK;
    }
    else {
        ModelApplyImageTexture(model, iMat);
        UnloadImage(iMat);
    }

    SetTargetFPS(TARGET_FPS);

    DisableCursor();
    int cursorLocked = 1;

    int playing = 1;

    unsigned frameCount = 0;
    if (datData)
        frameCount = DatGetFrameCount(datData);
    float currentFrame = 0.f;

    float animSpeed = 1.f;

    unsigned keyCount = 0;
    if (vdfData)
        keyCount = VdfGetKeyCount(vdfData);
    unsigned currentKey = 0;

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_U)) {
            if (cursorLocked) {
                EnableCursor();
                cursorLocked = 0;
            }
            else {
                DisableCursor();
                cursorLocked = 1;
            }
        }
        
        if (IsKeyPressed(KEY_J)) {
            if (onlyVdf) {
                if (currentKey == 0)
                    currentKey = keyCount - 1;
                else
                    currentKey--;

                ModelReset(model);
                ModelApplyVdf(model, vdfData, currentKey, 1.f);
                ModelUpdate(model);
            }
            else if (canAnimate)
                animSpeed -= .25f;
        }
        if (IsKeyPressed(KEY_K)) {
            if (onlyVdf) {
                if (currentKey >= keyCount - 1)
                    currentKey = 0;
                else
                    currentKey++;

                ModelReset(model);
                ModelApplyVdf(model, vdfData, currentKey, 1.f);
                ModelUpdate(model);
            }
            else if (canAnimate)
                animSpeed += .25f;
        }

        if (IsKeyPressed(KEY_T) && canAnimate)
            playing ^= true;

        if (playing && canAnimate) {
            currentFrame += (30.f / TARGET_FPS) * animSpeed;
            if (currentFrame >= frameCount)
                currentFrame = 0.f;
            if (currentFrame < 0.f)
                currentFrame = frameCount - 1;

            ModelReset(model);
            ModelApplyDatVdf(model, vdfData, datData, currentFrame);
            ModelUpdate(model);
        }

        if (cursorLocked)
            UpdateCamera(&camera, CAMERA_FREE);

		BeginDrawing();

            ClearBackground(WHITE);

            BeginBlendMode(BLEND_ALPHA);

            BeginMode3D(camera);

                if (!noTexture)
                    ModelSubmitDraw(model);
                else
                    ModelSubmitWireDraw(model);

                DrawGrid(20, 2.f);

            EndMode3D();

            EndBlendMode();

            char text[256];

            if (canAnimate) {
                DrawRectangle(15, WINDOW_HEIGHT - 40, WINDOW_WIDTH - 30, 25, (Color){ 0, 0, 0, 50 });

                float animProgress = currentFrame / frameCount;
                DrawRectangle(20, WINDOW_HEIGHT - 30, (WINDOW_WIDTH - 40) * animProgress, 10, (Color){ 30, 55, 255, 200 });

                sprintf(text, "current frame : %u/%u", (unsigned)currentFrame+1, frameCount);
                DrawText(text, 0, 0, 20, BLACK);

                sprintf(text, "speed : %fx (press J/K)", animSpeed);
                DrawText(text, 0, 20, 20, BLACK);

                sprintf(text, "playing : %s (press T)", playing ? "yes" : "no");
                DrawText(text, 0, 40, 20, BLACK);

                sprintf(text, "cursor : %s (press U)", cursorLocked ? "locked" : "unlocked");
                DrawText(text, 0, 60, 20, BLACK);
            }
            else if (onlyVdf) {
                sprintf(text, "current key : %u/%u (press J/K)", (unsigned)currentKey+1, keyCount);
                DrawText(text, 0, 0, 20, BLACK);

                sprintf(text, "cursor : %s (press U)", cursorLocked ? "locked" : "unlocked");
                DrawText(text, 0, 20, 20, BLACK);
            }
            else {
                sprintf(text, "cursor : %s (press U)", cursorLocked ? "locked" : "unlocked");
                DrawText(text, 0, 0, 20, BLACK);
            }

		EndDrawing();
	}

    CloseWindow();

    // Cleanup

    free(args.timFiles);

    ModelDestroy(model);
    free(tmdData);

    if (vdfData)
        free(vdfData);
    if (datData)
        free(datData);

    printf("\nAll done. Exiting..\n");

    return 0;
}