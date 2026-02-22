#pragma once
#include <vector>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../Scene/Geometry/Triangle.h"
#include "../Primitives/Mesh.h"
#include <iostream>
#include <fstream>
#include <glm/glm.hpp>

class MeshLoader
{
public:
    static std::vector<Mesh*> LoadMesh(const std::string& path);

private:
    static void ProcessNode(aiNode *node, const aiScene *scene, std::vector<Mesh*>& parts);

    static void ProcessMesh(aiMesh *mesh, const aiScene *scene, std::vector<Mesh*>& parts);
};
