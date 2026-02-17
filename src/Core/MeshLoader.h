#pragma once
#include <vector>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../Scene/Geometry/Triangle.h"
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>

struct MeshPart
{
    std::string materialName;
    std::vector<Triangle> triangles;
};

class MeshLoader
{
public:
    static std::vector<MeshPart> LoadMesh(const std::string& path)
    {
        std::cout << "MeshLoader: Starting to load " << path << std::endl;
        
        Assimp::Importer importer;
        // aiProcess_Triangulate: Ensure all faces are triangles
        // aiProcess_FlipUVs: Flip Y coordinate of UVs if necessary (depends on renderer, usually needed for OpenGL/SFML if 0,0 is bottom-left vs top-left)
        // SFML 0,0 is top-left. OpenGL 0,0 is bottom-left.
        // Textures are loaded via SFML.
        // Let's assume we might need FlipUVs.
        // aiProcess_GenNormals: Generate normals if missing.
        // aiProcess_PreTransformVertices: Bake node transforms into vertices. Critical for OBJ/FBX to ensure correct position/scale.
        const aiScene* scene = importer.ReadFile(path, 
            aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices | aiProcess_FindDegenerates | aiProcess_PreTransformVertices);

        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
        {
            std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return {};
        }

        std::cout << "MeshLoader: Scene loaded from file. Processing nodes..." << std::endl;
        std::vector<MeshPart> parts;
        ProcessNode(scene->mRootNode, scene, parts);
        
        size_t totalTriangles = 0;
        for (const auto& part : parts) totalTriangles += part.triangles.size();
        
        std::cout << "Loaded mesh " << path << " with " << totalTriangles << " triangles across " << parts.size() << " parts." << std::endl;
        
        return parts;
    }

private:
    static void ProcessNode(aiNode *node, const aiScene *scene, std::vector<MeshPart>& parts)
    {
        // process all the node's meshes (if any)
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
            ProcessMesh(mesh, scene, parts);
        }
        // then do the same for each of its children
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(node->mChildren[i], scene, parts);
        }
    }

    static void ProcessMesh(aiMesh *mesh, const aiScene *scene, std::vector<MeshPart>& parts)
    {
        MeshPart part;
        
        // Get material name
        if (mesh->mMaterialIndex >= 0 && mesh->mMaterialIndex < scene->mNumMaterials)
        {
            aiString name;
            if (scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
            {
                part.materialName = name.C_Str();
            }
        }
        
        // Iterate over faces (triangles)
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            if(face.mNumIndices != 3) continue;

            // Get vertices
            aiVector3D p0 = mesh->mVertices[face.mIndices[0]];
            aiVector3D p1 = mesh->mVertices[face.mIndices[1]];
            aiVector3D p2 = mesh->mVertices[face.mIndices[2]];

            // Assimp uses generic float vectors. We convert to GLM.
            fvec3 v0(p0.x, p0.y, p0.z);
            fvec3 v1(p1.x, p1.y, p1.z);
            fvec3 v2(p2.x, p2.y, p2.z);

            // Create Triangle with default white color
            Triangle t(v0, v1, v2, fvec3(1.0f)); 

            // Get UVs if available
            if(mesh->mTextureCoords[0]) 
            {
                aiVector3D uv0 = mesh->mTextureCoords[0][face.mIndices[0]];
                aiVector3D uv1 = mesh->mTextureCoords[0][face.mIndices[1]];
                aiVector3D uv2 = mesh->mTextureCoords[0][face.mIndices[2]];

                t.SetUVs(fvec2(uv0.x, uv0.y), fvec2(uv1.x, uv1.y), fvec2(uv2.x, uv2.y));
            }
            
            part.triangles.push_back(t);
        }
        
        parts.push_back(part);
    }
};
