#ifndef MODEL_H
#define MODEL_H

#include "tmdProcess.h"

#include "vdfProcess.h"
#include "datProcess.h"

#include <raylib.h>
#include <raymath.h>

#include "common.h"

#define MODEL_SCALE (.02f)

const char MAT_SHADER[] =
"#version 330\n"
"in vec2 fragTexCoord;\n"
"in vec4 fragColor;\n"
"uniform sampler2D texture0;\n"
"uniform vec4 colDiffuse;\n"
"out vec4 finalColor;\n"
"void main() {\n"
"    vec4 texelColor = texture(texture0, fragTexCoord) * colDiffuse;\n"
"    if (texelColor.a < 0.1) discard;\n"
"    finalColor = texelColor;\n"
"}";

typedef struct {
    u8* _tmdDataOriginal; // Original TMD data
    u32 _tmdDataSize; // Size of orignal TMD data
    u8* _tmdData; // Mutable copy of original TMD data

    Model* rModel;

    Vector3 position;

    Vector3 rotationAxis;
    float rotationAngle;
    
    Vector3 scale;

    Color tint;
} ModelData;

void _ModelFillMesh(ModelData* model, unsigned objectIndex) {
    u32 primitiveCount = TmdObjectGetPrimitiveCount(model->_tmdData, objectIndex);
    WorkPrimitive* primitives = TmdObjectCreateWorkPrimitives(model->_tmdData, objectIndex);

    Mesh* mesh = model->rModel->meshes + objectIndex;

    unsigned vertexOffset = 0;
    unsigned indexOffset = 0;

    for (unsigned i = 0; i < primitiveCount; i++) {
        WorkPrimitive* prim = primitives + i;

        if (!prim->flags.OK)
            continue;

        if (prim->flags.isLine) {
            // Line case: Use only v1 and v2

            for (unsigned j = 0; j < 2; j++) { // Process only two vertices (v1, v2)
                unsigned vertexIndex = (vertexOffset + j) * 3;
                mesh->vertices[vertexIndex + 0] = (float)prim->vertices[j][0]; // X
                mesh->vertices[vertexIndex + 1] = (float)prim->vertices[j][1]; // Y
                mesh->vertices[vertexIndex + 2] = (float)prim->vertices[j][2]; // Z

                // Fill normals
                mesh->normals[vertexIndex + 0] = (float)prim->normals[j][0]; // Normal X
                mesh->normals[vertexIndex + 1] = (float)prim->normals[j][1]; // Normal Y
                mesh->normals[vertexIndex + 2] = (float)prim->normals[j][2]; // Normal Z

                // Fill color based on vertex
                unsigned colorIndex = (vertexOffset + j) * 4;
                if (j == 0) {
                    mesh->colors[colorIndex + 0] = prim->rgb0[0];
                    mesh->colors[colorIndex + 1] = prim->rgb0[1];
                    mesh->colors[colorIndex + 2] = prim->rgb0[2];
                    mesh->colors[colorIndex + 3] = 255; // Alpha
                } else {
                    mesh->colors[colorIndex + 0] = prim->rgb1[0];
                    mesh->colors[colorIndex + 1] = prim->rgb1[1];
                    mesh->colors[colorIndex + 2] = prim->rgb1[2];
                    mesh->colors[colorIndex + 3] = 255; // Alpha
                }
            }

            // Fill indices for the line (2 indices)
            mesh->indices[indexOffset + 0] = vertexOffset + 0; // v1
            mesh->indices[indexOffset + 1] = vertexOffset + 1; // v2

            vertexOffset += 2;
            indexOffset += 2;

        }
        else {
            // Triangle case: Use all 3 vertices (v1, v2, v3)

            for (unsigned j = 0; j < 3; j++) {
                unsigned vertexIndex = (vertexOffset + j) * 3;
                mesh->vertices[vertexIndex + 0] = (float)(prim->vertices[j][0]); // X
                mesh->vertices[vertexIndex + 1] = (float)(prim->vertices[j][1]); // Y
                mesh->vertices[vertexIndex + 2] = (float)(prim->vertices[j][2]); // Z

                // Fill normals
                mesh->normals[vertexIndex + 0] = prim->normals[j][0]; // Normal X
                mesh->normals[vertexIndex + 1] = prim->normals[j][1]; // Normal Y
                mesh->normals[vertexIndex + 2] = prim->normals[j][2]; // Normal Z

                // Fill color based on vertex
                unsigned colorIndex = (vertexOffset + j) * 4;
                if (!prim->flags.isTextured)
                    switch (j) {
                        case 0:
                            mesh->colors[colorIndex + 0] = prim->rgb0[0];
                            mesh->colors[colorIndex + 1] = prim->rgb0[1];
                            mesh->colors[colorIndex + 2] = prim->rgb0[2];
                            mesh->colors[colorIndex + 3] = 255; // Alpha
                            break;
                        case 1:
                            mesh->colors[colorIndex + 0] = prim->rgb1[0];
                            mesh->colors[colorIndex + 1] = prim->rgb1[1];
                            mesh->colors[colorIndex + 2] = prim->rgb1[2];
                            mesh->colors[colorIndex + 3] = 255; // Alpha
                            break;
                        case 2:
                            mesh->colors[colorIndex + 0] = prim->rgb2[0];
                            mesh->colors[colorIndex + 1] = prim->rgb2[1];
                            mesh->colors[colorIndex + 2] = prim->rgb2[2];
                            mesh->colors[colorIndex + 3] = 255; // Alpha
                            break;
                    }
                else {
                    mesh->colors[colorIndex + 0] = 255;
                    mesh->colors[colorIndex + 1] = 255;
                    mesh->colors[colorIndex + 2] = 255;
                    mesh->colors[colorIndex + 3] = 255;
                }

                if (prim->flags.isTextured) {
                    unsigned coordIndex = (vertexOffset + j) * 2;
                    switch (j) {
                        case 0:
                            mesh->texcoords[coordIndex + 0] = prim->uv0[0] / (float)VR_WIDTH32;
                            mesh->texcoords[coordIndex + 1] = prim->uv0[1] / (float)VR_HEIGHT;
                            break;
                        case 1:
                            mesh->texcoords[coordIndex + 0] = prim->uv1[0] / (float)VR_WIDTH32;
                            mesh->texcoords[coordIndex + 1] = prim->uv1[1] / (float)VR_HEIGHT;
                            break;
                        case 2:
                            mesh->texcoords[coordIndex + 0] = prim->uv2[0] / (float)VR_WIDTH32;
                            mesh->texcoords[coordIndex + 1] = prim->uv2[1] / (float)VR_HEIGHT;
                            break;
                    }
                }
            }

            // Fill indices for the triangle (3 indices)
            mesh->indices[indexOffset + 0] = vertexOffset + 0; // v1
            mesh->indices[indexOffset + 1] = vertexOffset + 1; // v2
            mesh->indices[indexOffset + 2] = vertexOffset + 2; // v3

            vertexOffset += 3;
            indexOffset += 3;
        }
    }
}

ModelData* ModelCreate(u8* tmdData, u32 tmdDataSize) {
    ModelData* model = (ModelData*)malloc(sizeof(ModelData));

    TmdPreprocess(tmdData);

    model->_tmdDataOriginal = tmdData;
    model->_tmdDataSize = tmdDataSize;

    model->_tmdData = (u8*)malloc(tmdDataSize);
    memcpy(model->_tmdData, tmdData, tmdDataSize);

    model->rModel = (Model*)malloc(sizeof(Model));
    *model->rModel = (Model){ 0 };

    {
        model->rModel->meshCount = TmdGetObjectCount(model->_tmdData);
        model->rModel->meshes = (Mesh*)calloc(model->rModel->meshCount, sizeof(Mesh));

        for (unsigned m = 0; m < model->rModel->meshCount; m++) {
            u32 primitiveCount = TmdObjectGetPrimitiveCount(model->_tmdData, m);
            WorkPrimitive* primitives = TmdObjectCreateWorkPrimitives(model->_tmdData, m);

            Mesh* mesh = model->rModel->meshes + m;
            
            unsigned totalVertices = 0;
            unsigned totalIndices = 0;

            for (unsigned i = 0; i < primitiveCount; i++) {
                // Lines only use 2 vertices
                if (primitives[i].flags.isLine) {
                    totalVertices += 2;
                    totalIndices += 2;
                }
                else {
                    totalVertices += 3;
                    totalIndices += 3;
                }
            }

            mesh->vertexCount = totalVertices;
            mesh->vertices = (float*)malloc(mesh->vertexCount * 3 * sizeof(float));
            mesh->normals = (float*)malloc(mesh->vertexCount * 3 * sizeof(float));
        
            mesh->colors = (u8*)malloc(mesh->vertexCount * 4);
            mesh->triangleCount = totalIndices / 3; // Each triangle is 3 indices, lines count as separate
            mesh->indices = (unsigned short*)malloc(totalIndices * sizeof(unsigned short));

            mesh->texcoords = (float*)malloc(mesh->vertexCount * 2 * sizeof(float));

            _ModelFillMesh(model, m);

            UploadMesh(mesh, 1);
        }

        model->rModel->transform = MatrixScale(-MODEL_SCALE, -MODEL_SCALE, -MODEL_SCALE);
    }

    model->position = (Vector3){ 0.f, 0.f, 0.f };

    model->rotationAxis = (Vector3){ 0.f, 0.f, 0.f };
    model->rotationAngle = 0.f;

    model->scale = (Vector3){ 1.f, 1.f, 1.f };

    model->tint = WHITE;

    return model;
}

// Reset internal TMD model. ModelUpdate must be called before changes are reflected
void ModelReset(ModelData* model) {
    memcpy(model->_tmdData, model->_tmdDataOriginal, model->_tmdDataSize);
}

// Assumes vertex & normal count have not changed. Does not realloc
void ModelUpdate(ModelData* model) {
    for (unsigned m = 0; m < model->rModel->meshCount; m++) {
        _ModelFillMesh(model, m);

        Mesh* mesh = model->rModel->meshes + m;

        UpdateMeshBuffer(*mesh, 0, mesh->vertices, mesh->vertexCount * 3 * sizeof(float), 0);
    }
}

// Frees model ptr
void ModelDestroy(ModelData* model) {
    for (unsigned m = 0; m < model->rModel->meshCount; m++)
        UnloadMesh(model->rModel->meshes[m]);
    for (unsigned m = 0; m < model->rModel->materialCount; m++)
        UnloadMaterial(model->rModel->materials[m]);

    free(model->_tmdData);

    free(model);
}

// Apply Vdf data from Dat
void ModelApplyDatVdf(ModelData* model, u8* vdfData, u8* datData, float frameNo) {
    TmdVertex* vertices = TmdObjectGetVertices(model->_tmdData, 0);

    DatApplyVdf(datData, vdfData, vertices, frameNo);
}

// Directly apply Vdf keyframe
void ModelApplyVdf(ModelData* model, u8* vdfData, u32 keyIndex, float influence) {
    u32 objectIndex = VdfGetKeyObjectIndex(vdfData, keyIndex);
    TmdVertex* vertices = TmdObjectGetVertices(model->_tmdData, objectIndex);

    VdfApply(vdfData, keyIndex, influence, vertices);
}

void ModelApplyDefaultMaterial(ModelData* model) {
    if (model->rModel->materials)
        free(model->rModel->materials);
    if (model->rModel->meshMaterial)
        free(model->rModel->meshMaterial);

    model->rModel->materialCount = 1;
    model->rModel->materials = (Material *)calloc(1, sizeof(Material));

    model->rModel->materials[0] = LoadMaterialDefault();

    model->rModel->meshMaterial = (int*)calloc(model->rModel->meshCount, sizeof(int));
}

// Directly apply texture to model (Texture2D)
void ModelApplyTexture(ModelData* model, Texture2D texture) {
    if (model->rModel->materials)
        free(model->rModel->materials);
    if (model->rModel->meshMaterial)
        free(model->rModel->meshMaterial);

    model->rModel->materialCount = 1;
    model->rModel->materials = (Material *)calloc(1, sizeof(Material));

    model->rModel->materials[0] = LoadMaterialDefault();
    model->rModel->materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    model->rModel->materials[0].shader = LoadShaderFromMemory(0, MAT_SHADER);

    model->rModel->meshMaterial = (int*)calloc(model->rModel->meshCount, sizeof(int));
}

// Apply texture to model from Image
void ModelApplyImageTexture(ModelData* model, Image image) {
    Texture2D texture = LoadTextureFromImage(image);

    ModelApplyTexture(model, texture);
}

void ModelSubmitDraw(ModelData* model) {
    DrawModelEx(*model->rModel, model->position, model->rotationAxis, model->rotationAngle, model->scale, model->tint);
}
void ModelSubmitWireDraw(ModelData* model) {
    DrawModelWiresEx(*model->rModel, model->position, model->rotationAxis, model->rotationAngle, model->scale, model->tint);
}

#endif
